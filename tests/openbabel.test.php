<?php

/**
 * @file
 * Test class for CLI adapter.
 *
 * We assume that OpenBabel has been installed in /usr/bin/obabel
 */

require_once __DIR__ . '/../includes/commands/openbabel.inc';

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

  /**
   * Get chemical fixtures/samples.
   */
  protected function getSamples() {
    $files = array_map(array($this, 'addDirectoryPrefix'), scandir(__DIR__ . '/fixtures/chemicals'));
    $dirs = array_filter($files, 'is_dir');
    $bad = array_map(array($this, 'addDirectoryPrefix'), array(
      // Unsupported as OpenBabel input.
      'example.inp',
    ));
    return array_diff($files, $dirs, $bad);
  }

  /**
   * PHP's scandir() is silly, and doesn't prefix files... Let's.
   */
  protected function addDirectoryPrefix($file) {
    return __DIR__ . "/fixtures/chemicals/$file";
  }

  /**
   * Test InchiKey output for separate molecules, as used in D6 code.
   *
   * @see https://github.com/discoverygarden/islandora_solution_pack_chemistry/blob/453a885b70c1396a807b4160dcbb2426c8ec1875/chem.inc#L432
   */
  public function testOutputSeparateInchi() {
    $options = new \Islandora\Chemistry\OpenBabel\Options(array(
      'o' => 'inchi',
      'separate' => TRUE,
      'x' => 'K',
    ));
    $no_separate = array(
      // Seems to break when trying to separate...
      'example.mop',
    );
    foreach (array_diff($this->getSamples(), array_map(array($this, 'addDirectoryPrefix'), $no_separate)) as $fixture) {
      $result = \Islandora\Chemistry\OpenBabel\execute(
        $fixture,
        $options,
        '/usr/bin/obabel'
      );
      if (!empty($result)) {
        foreach (explode("\n", $result) as $key) {
          $this->assertRegExp('/^.{14}-.{9}.-.$/', $key, "Bad key from $fixture.");
        }
      }
    }
  }

  /**
   * Test InchiKey output, as used in D6 code.
   *
   * @see https://github.com/discoverygarden/islandora_solution_pack_chemistry/blob/453a885b70c1396a807b4160dcbb2426c8ec1875/islandora_chem_sp_search.inc#L112
   * @see https://github.com/discoverygarden/islandora_solution_pack_chemistry/blob/453a885b70c1396a807b4160dcbb2426c8ec1875/chem.inc#L435
   */
  public function testOutputInchi() {
    // exec("obabel \"$file\" -oinchi -xK", $inchi_key)
    // exec("obabel \"$file\" -oinchi -xK", $inchi_key, $returnValue2)
    $options = new \Islandora\Chemistry\OpenBabel\Options(array(
      'o' => 'inchi',
      'x' => 'K',
    ));
    foreach ($this->getSamples() as $fixture) {
      $result = \Islandora\Chemistry\OpenBabel\execute(
        $fixture,
        $options,
        '/usr/bin/obabel'
      );
      if (!empty($result)) {
        foreach (explode("\n", $result) as $key) {
          $this->assertRegExp('/^.{14}-.{9}.-.$/', $key, "Bad key from $fixture.");
        }
      }
    }
  }

  /**
   * Test output of MOL files, as used in D6 code.
   *
   * @see https://github.com/discoverygarden/islandora_solution_pack_chemistry/blob/453a885b70c1396a807b4160dcbb2426c8ec1875/chem.inc#L345
   * @see https://github.com/discoverygarden/islandora_solution_pack_chemistry/blob/453a885b70c1396a807b4160dcbb2426c8ec1875/chem.inc#L441
   * @see https://github.com/discoverygarden/islandora_solution_pack_chemistry/blob/453a885b70c1396a807b4160dcbb2426c8ec1875/islandora_chem_sp_search.inc#L135
   * @see https://github.com/discoverygarden/islandora_solution_pack_chemistry/blob/453a885b70c1396a807b4160dcbb2426c8ec1875/islandora_chem_sp_search.inc#L161
   */
  public function testOutputMol() {
    $temp_file = $this->tmp();
    $options = new \Islandora\Chemistry\OpenBabel\Options(array(
      'o' => 'mol',
      'O' => $temp_file,
    ));
    foreach ($this->getSamples() as $fixture) {
      $result = \Islandora\Chemistry\OpenBabel\execute(
        $fixture,
        $options,
        '/usr/bin/obabel'
      );
      $this->assertFileExists($temp_file, 'Mol file was created.');
    }

    $this->tmps[] = $temp_file = tempnam(sys_get_temp_dir(), 'mol');
    $options = new \Islandora\Chemistry\OpenBabel\Options(array(
      'title' => '',
      'o' => 'mol',
      'O' => $temp_file,
      'x' => 'w',
    ));
    foreach ($this->getSamples() as $fixture) {
      $result = \Islandora\Chemistry\OpenBabel\execute(
        $fixture,
        $options,
        '/usr/bin/obabel'
      );
      $this->assertFileExists($temp_file, 'Mol file was created.');
    }
  }

  /**
   * Test output of PNGs, as used in D6 code.
   *
   * @see https://github.com/discoverygarden/islandora_solution_pack_chemistry/blob/453a885b70c1396a807b4160dcbb2426c8ec1875/chem.inc#L381
   */
  public function testOutputPng() {
    $temp_file = $this->tmp();
    $options = new \Islandora\Chemistry\OpenBabel\Options(array(
      'o' => 'png',
      'O' => $temp_file,
      'x' => 'w',
      'c' => TRUE,
    ));
    foreach ($this->getSamples() as $fixture) {
      $result = \Islandora\Chemistry\OpenBabel\execute(
        $fixture,
        $options,
        '/usr/bin/obabel'
      );
      $this->assertFileExists($temp_file, 'Png file was created.');
    }
  }

  /**
   * Test output of SMI, as used in D6 code.
   *
   * @see https://github.com/discoverygarden/islandora_solution_pack_chemistry/blob/453a885b70c1396a807b4160dcbb2426c8ec1875/chem.inc#L460
   */
  public function testOutputSmi() {
    $temp_file = $this->tmp();
    $options = new \Islandora\Chemistry\OpenBabel\Options(array(
      'o' => 'smi',
      'O' => $temp_file,
      'x' => 'c',
    ));
    foreach ($this->getSamples() as $fixture) {
      $result = \Islandora\Chemistry\OpenBabel\execute(
        $fixture,
        $options,
        '/usr/bin/obabel'
      );
      $this->assertFileExists($temp_file, 'SMI file was created.');
    }
  }

  /**
   * Test that the standard input method appears to work.
   *
   * @dataProvider stdInFixtures
   */
  public function testStandardInputMethod($input_format, $mol_file) {
    $temp_file = $this->tmp();
    $options = new \Islandora\Chemistry\OpenBabel\Options(array(
      'o' => 'cml',
      'O' => $temp_file,
    ));
    if ($input_format) {
      $options['i'] = $input_format;
    }
    $result = \Islandora\Chemistry\OpenBabel\execute(
      $mol_file,
      $options,
      '/usr/bin/obabel'
    );
    $this->assertFileExists($temp_file, 'CML file was created.');
    $this->assertGreaterThan(0, filesize($temp_file));
  }

  /**
   * Data provider to get test info for stdin.
   */
  public function stdInFixtures() {
    return array(
      array(FALSE, __DIR__ . '/fixtures/chemicals/example.mol'),
      array('mol', __DIR__ . '/fixtures/chemicals/example.mol'),
      array('cml', __DIR__ . '/fixtures/chemicals/example.cml'),
    );
  }
}
