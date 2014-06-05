<?php

/**
 * @file
 * Test class for CLI adapter.
 *
 * We assume that cmmmsrv is available on localhost at 55624.
 */

require_once __DIR__ . '/../includes/checkmol.inc';

class OpenBabelTest extends PHPUnit_Framework_TestCase {
  /**
   * Inherits.
   */
  public function setUp() {
    $this->tmps = array();
  }

  /**
   * Inherits.
   */
  public function tearDown() {
    foreach ($this->tmps as $tmp) {
      @unlink($tmp);
    }
  }

  /**
   * Temp file helper.
   */
  protected function tmp() {
    $this->tmps[] = $tmp = tempnam(sys_get_temp_dir(), 'chem_test');
    unlink($tmp);
    return $tmp;
  }

  public function testCheckmol8DigitCode() {
    $cm = new \Islandora\Chemistry\Checkmol();
    $output = $cm->get8DigitCodes(__DIR__ . '/fixtures/chemicals/example.mol');
    print_r($output);
  }
}
