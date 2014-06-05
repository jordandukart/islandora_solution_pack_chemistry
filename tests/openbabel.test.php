<?php

/**
 * @file
 * Test class for CLI adapter.
 *
 * We assume that OpenBabel has been installed in /usr/bin/obabel
 */

require_once __DIR__ . '/../includes/commands/openbabel.inc';
require_once __DIR__ . '/chem.test.inc';

class OpenBabelTest extends IslandoraChemistryUnitTest {
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
    foreach ($this->getSamples() as $fixture) {
      $temp_file = $this->tmp();
      $options = new \Islandora\Chemistry\OpenBabel\Options(array(
        'o' => 'mol',
        'O' => $temp_file,
      ));

      $result = \Islandora\Chemistry\OpenBabel\execute(
        $fixture,
        $options,
        '/usr/bin/obabel'
      );
      $this->assertFileExists($temp_file, 'Mol file was created.');
      $this->assertGreaterThan(0, filesize($temp_file), "Mol file from $fixture is not empty.");
      unlink($temp_file);
    }

    foreach ($this->getSamples() as $fixture) {
      $temp_file = $this->tmp();
      $options = new \Islandora\Chemistry\OpenBabel\Options(array(
        'title' => '',
        'o' => 'mol',
        'O' => $temp_file,
        'x' => 'w',
      ));

      $result = \Islandora\Chemistry\OpenBabel\execute(
        $fixture,
        $options,
        '/usr/bin/obabel'
      );
      $this->assertFileExists($temp_file, 'Mol file was created.');
      $this->assertGreaterThan(0, filesize($temp_file), "Mol file from $fixture is not empty.");
      unlink($temp_file);
    }
    return $this->tmps;
  }

  /**
   * Test output of PNGs, as used in D6 code.
   *
   * PNGs do not seem to be images?
   *
   * @see https://github.com/discoverygarden/islandora_solution_pack_chemistry/blob/453a885b70c1396a807b4160dcbb2426c8ec1875/chem.inc#L381
   */
  public function testOutputPng() {
    $no_png = array(
      // Seems to break when trying to generate an image...
      'example.pdb',
    );
    foreach (array_diff($this->getSamples(), array_map(array($this, 'addDirectoryPrefix'), $no_png)) as $fixture) {
      $temp_file = $this->tmp();
      $options = new \Islandora\Chemistry\OpenBabel\Options(array(
        'o' => 'png',
        'O' => $temp_file,
        'x' => 'w',
        'c' => TRUE,
      ));
      $result = \Islandora\Chemistry\OpenBabel\execute(
        $fixture,
        $options,
        '/usr/bin/obabel'
      );
      $this->assertFileExists($temp_file, 'Png file was created.');
      $this->assertGreaterThan(0, filesize($temp_file), "Png file from $fixture is not empty.");
      unlink($temp_file);
    }
  }

  /**
   * Test output of SMI, as used in D6 code.
   *
   * @see https://github.com/discoverygarden/islandora_solution_pack_chemistry/blob/453a885b70c1396a807b4160dcbb2426c8ec1875/chem.inc#L460
   */
  public function testOutputSmi() {
    $no_smi = array(
      // Seems to break when trying to generate an image...
      'example.pdb',
    );
    foreach (array_diff($this->getSamples(), array_map(array($this, 'addDirectoryPrefix'), $no_smi)) as $fixture) {
      $temp_file = $this->tmp();
      $options = new \Islandora\Chemistry\OpenBabel\Options(array(
        'o' => 'smi',
        'O' => $temp_file,
        'x' => 'c',
      ));
      $result = \Islandora\Chemistry\OpenBabel\execute(
        $fixture,
        $options,
        '/usr/bin/obabel'
      );
      $this->assertFileExists($temp_file, 'SMI file was created.');
      $this->assertGreaterThan(0, filesize($temp_file), "Smi file from $fixture is not empty.");
      unlink($temp_file);
    }
  }
}
