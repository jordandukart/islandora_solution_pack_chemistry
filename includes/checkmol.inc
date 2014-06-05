<?php

/**
 * @file
 * Class related to socket-based comms with cmmmsrv (Checkmol server).
 */

namespace Islandora\Chemistry;
use Exception,
    InvalidArgumentException;

class Checkmol {
  /**
   * @var $socket
   *   File-pointer for the socket.
   */
  protected $socket;

  /**
   * @var $ready
   *   Flag to indicate if we have gotta the intro message from the server.
   */
  protected $ready = FALSE;

  /**
   * @const HASHES
   *   String used various places in comms... Often to start messages.
   */
  const HASHES = '####';

  /**
   * Constructor.
   *
   * @param string $host
   *   The address to which to connect. Defaults to 'localhost'.
   * @param int $port
   *   The port to which to connect on the given host. Defaults to 55624.
   */
  public function __construct($host = 'localhost', $port = 55624) {
    $this->socket = fsockopen($host, $port);
  }

  /**
   * Destructor.
   *
   * Attempt to disconnect gracefully.
   */
  public function __destruct() {
    $this->send(static::HASHES . " BYE\n");
    fclose($this->socket);
  }

  /**
   * Determine if the connection is ready for communications.
   */
  public function ready() {
    // Assume that if the first for bytes are '####', the socket is ready to
    // go.
    if (!$this->ready) {
      $buffer = fgets($this->socket);
      $this->ready = strpos($buffer, static::HASHES) === 0;
    }

    return $this->ready;
  }

  /**
   * Convenience method to send a message over the connection.
   *
   * Appends a newline "\n" to the end of the message, so it is interpreted as
   * a command.
   *
   * @see Islandora\Chemistry\Checkmol\sendRaw()
   */
  public function send($message) {
    $entered_message = "$message\n";
    $this->sendRaw($entered_message);
  }

  /**
   * Send a message over the connection.
   *
   * @param string $message
   *   The message to send over the connection.
   */
  protected function sendRaw($message) {
    while (!$this->ready()) {
      sleep(1);
    }
    fwrite($this->socket, $message);
  }

  /**
   * Read a line from the connection.
   *
   * @return string
   *   A line read from the connection (ending in a newline).
   */
  protected function readline() {
    while (!$this->ready()) {
      sleep(1);
    }
    return fgets($this->socket);
  }

  /**
   * Run a command against cmmmsrv.
   *
   * @param string $command
   *   A command to run. See the documentation for cmmmsrv as to what this
   *   might actually be.
   * @param string $mol_input
   *   An optional molecule structure on which to run the command.
   *
   * @return array
   *   The lines of output generated from the given command.
   */
  protected function runCommand($command, $mol_input = FALSE) {
    $this->send(static::HASHES . " $command");
    if ($mol_input) {
      $this->send($mol_input);
      $this->send('$$$$');
    }
    $this->send(static::HASHES);
    $found_final_hashes = FALSE;
    $output = array();
    while (!$found_final_hashes) {
      $line = $this->readline();
      if ($line !== FALSE) {
        $output[] = $line;
      }
      $found_final_hashes = end($output) == static::HASHES . "\n";
      sleep(1);
    }

    return array_map('trim', array_slice($output, 0, -1));
  }

  /**
   * Convenience method; run a command against the given molecule structure.
   */
  public function runCommandWithFile($command, $mol_file) {
    return $this->runCommand($command, file_get_contents($mol_file));
  }

  /**
   * Get 8-digit codes for all fragments in a given MOL file.
   *
   * @see http://merian.pch.univie.ac.at/~nhaider/cheminf/fgtable.pdf
   *
   * @param string $mol_file
   *   A string containing the filename of a MOL file.
   *
   * @return array
   *   A (potentially empty) array of all fragments contained in the given
   *   file.
   *
   * @throws UnexpectedOutputException
   *   Thrown if there is more than one line output as a result of this
   *   command.
   * @throws InvalidArgumentException
   *   Thrown if the given molecule file was reported as being invalid by
   *   the checkmol command.
   */
  public function get8DigitCodes($mol_file) {
    $semi_trim = function ($str) {
      return rtrim($str, ';');
    };

    $command_output = $this->runCommandWithFile('checkmol:c', $mol_file);
    $filtered_output = array_filter(array_map($semi_trim, $command_output));
    if (count($filtered_output) > 1) {
      // Too many lines.
      throw new UnexpectedOutputException('Too many lines returned from command.');
    }
    $output = reset($filtered_output);
    if ($output == static::HASHES . ' invalid molecule') {
      throw new InvalidArgumentException("Bad molecule file input: $mol_file");
    }
    return explode(';', $output);
  }
}

class UnexpectedOutputException extends Exception {
}
