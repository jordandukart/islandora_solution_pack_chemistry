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

  protected $timeout = NULL;

  /**
   * Constructor.
   *
   * @param string $host
   *   The address to which to connect. Defaults to 'localhost'.
   * @param int $port
   *   The port to which to connect on the given host. Defaults to 55624.
   * @param float $timeout
   *   An amount of time in seconds for which to attempt to connect. We use
   *   PHP's "default_socket_timeout" for a default.
   */
  public function __construct($host = 'localhost', $port = 55624, $timeout = NULL) {
    $this->timeout = $timeout;
    if ($this->timeout !== NULL) {
      $error = $error_string = '';
      $this->socket = fsockopen($host, $port, $error, $error_string, $this->timeout);
    }
    else {
      $this->socket = fsockopen($host, $port);
    }

    if ($this->socket === FALSE) {
      throw new IOException('Failed to connect.');
    }
    elseif ($this->timeout !== NULL) {
      $this->send(static::HASHES);
      stream_set_timeout($this->socket, $this->timeout);
    }
  }

  /**
   * Destructor.
   *
   * Attempt to disconnect gracefully.
   */
  public function __destruct() {
    if ($this->socket !== FALSE) {
      $this->send(static::HASHES . ' BYE');
      fclose($this->socket);
    }
  }

  /**
   * Determine if the connection is ready for communications.
   */
  public function ready() {
    // Assume that if the first for bytes are '####', the socket is ready to
    // go.
    if (!$this->ready && $this->socket) {
      if ($this->timeout !== NULL) {
        $read = array($this->socket);
        $write = array();
        $except = array();
        $count = stream_select($read, $write, $except, $this->timeout);
        if (empty($read) || $count === 0) {
          return FALSE;
        }
      }
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
    if ($this->ready()) {
      fwrite($this->socket, $message);
    }
    else {
      throw new IOException('Not ready to send...');
    }
  }

  /**
   * Read a line from the connection.
   *
   * @return string
   *   A line read from the connection (ending in a newline).
   */
  protected function readline() {
    if ($this->ready()) {
      return fgets($this->socket);
    }
    else {
      throw new IOException('Not ready to recieve...');
    }
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
   *
   * @param string $command
   *   As string indicating the command to run, as accepted by runCommand().
   * @param string $mol
   *   A string containing either the filename of a MOL file, or the structure
   *   one might contain.
   */
  public function runCommandOnFileOrStructure($command, $mol) {
    return file_exists($mol) ?
      $this->runCommand($command, file_get_contents($mol)) :
      $this->runCommand($command, $mol);
  }

  /**
   * Get 8-digit codes for all fragments in a given MOL file.
   *
   * @see http://merian.pch.univie.ac.at/~nhaider/cheminf/fgtable.pdf
   *
   * @param string $mol
   *   A string containing either the filename of a MOL file, or the structure
   *   one might contain.
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
  public function get8DigitCodes($mol) {
    $semi_trim = function ($str) {
      return rtrim($str, ';');
    };

    $command_output = $this->runCommandOnFileOrStructure('checkmol:c', $mol);
    $filtered_output = array_filter(array_map($semi_trim, $command_output));
    if (count($filtered_output) > 1) {
      // Too many lines.
      throw new UnexpectedOutputException('Too many lines returned from command.');
    }
    $output = reset($filtered_output);
    if ($output == static::HASHES . ' invalid molecule') {
      throw new InvalidArgumentException("Bad molecule input: $mol");
    }
    return explode(';', $output);
  }
}

class UnexpectedOutputException extends Exception {
}

class IOException extends Exception {
}
