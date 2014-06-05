<?php

namespace Islandora\Chemistry;
use Exception;

class Checkmol {
  protected $socket;
  protected $buffer = '';
  protected $ready = FALSE;
  protected $blank = '####';

  public function __construct($host = 'localhost', $port = 55624) {
    $this->socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
    try {
      if (!socket_connect($this->socket, $host, $port)) {
        throw new Exception('Failed to open socket.');
      }
    }
    catch (Exception $e) {
      socket_close($this->socket);
      throw $e;
    }
    print_r(socket_last_error());
  }

  public function __destruct() {
    $this->send("#### BYE\n");
    socket_close($this->socket);
  }

  public function ready() {
    // Assume that if the first for bytes are '####', the socket is ready to
    // go.
    if (!$this->ready) {
      $buffer = '';
      socket_recv($this->socket, $buffer, 4096, 0);
      print_r("Blah: '$buffer'");
      $this->ready = strpos($buffer, '####') === 0;
    }

    return $this->ready;
  }

  public function send($message) {
    $entered_message = "$message\n";
    $this->sendRaw($entered_message);
  }

  protected function sendRaw($message) {
    $sent = 0;
    $length = strlen($message);
    while ($sent !== $length) {
      $sent += socket_send($this->socket, substr($message, $sent), $length - $sent, 0);
      print_r("Sending... $sent/$length\n");
    }
  }

  protected function readline() {
    $buffer = '';
    $length = socket_recv($this->socket, $buffer, 4096, MSG_DONTWAIT);
    $this->buffer .= $buffer;
    $pos = strpos($this->buffer, "\n");
    if ($pos !== FALSE) {
      $to_return = substr($this->buffer, 0, $pos);
      $this->buffer = substr($this->buffer, $pos);
      return $to_return;
    }
    return FALSE;
  }

  public function runCommand($command, $mol_file) {
    while (!$this->ready()) {
      print('loop of doom');
      sleep(1);
    }
    //return FALSE;
    $this->send("#### $command");
    $this->send(file_get_contents($mol_file));
    $this->send('$$$$');
    $this->send($this->blank);
    $buffer = '';
    $found_final_hashes = FALSE;
    $output = array();
    $i = 0;
    while (!$found_final_hashes && $i < 5) {
      $output[] = $this->readline();
      $found_final_hashes = end($output) == '####';
      sleep(1);
      print_r('asdf');
      print_r($output);
      $i++;
    }

    return implode("\n", array_slice($output, 0, -1));
  }

  public function get8DigitCodes($mol_file) {
    $output = $this->runCommand('checkmol:c', $mol_file);
    return $output;
  }
}
