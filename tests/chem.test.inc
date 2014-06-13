<?php

/**
 * @file
 * Base class for utility tests.
 */

abstract class IslandoraChemistryUnitTest extends PHPUnit_Framework_TestCase {
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

  /**
   * Get chemical fixtures/samples.
   */
  protected function getSamples() {
    $files = array_map(array($this, 'addDirectoryPrefix'), scandir(__DIR__ . '/fixtures/chemicals'));
    $dirs = array_filter($files, 'is_dir');
    $bad = array_map(array($this, 'addDirectoryPrefix'), array(
      // Unsupported as OpenBabel input.
      'example.inp',
      // Fails to output Mols and Pngs.
      'channel_5.dat',
      'channel_5.pos',
      'example.c3d',
      'example.chm',
      'example.dx',
      'example.gau',
      'example.jdx',
    ));
    return array_diff($files, $dirs, $bad);
  }

  /**
   * PHP's scandir() is silly, and doesn't prefix files... Let's.
   */
  protected function addDirectoryPrefix($file) {
    return __DIR__ . "/fixtures/chemicals/$file";
  }
}
