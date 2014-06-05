<?php

/**
 * @file
 * Test class for CLI adapter.
 *
 * We assume that cmmmsrv is available on localhost at 55624.
 */

require_once __DIR__ . '/../includes/checkmol.inc';
require_once __DIR__ . '/chem.test.inc';

class CheckmolTest extends IslandoraChemistryUnitTest {
  /**
   * Helper to provide mol files based on transforming the fixtures.
   */
  public function getMolFiles() {
    require_once __DIR__ . '/../includes/commands/openbabel.inc';
    $mol_files = array();

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
      $mol_files[] = array($temp_file);
    }
    return $mol_files;
  }

  /**
   * Test generation of fragment codes.
   *
   * @dataProvider getMolFiles
   */
  public function testCheckmol8DigitCode($mol_file) {
    $cm = new \Islandora\Chemistry\Checkmol();
    try {
      $output = $cm->get8DigitCodes($mol_file);
      foreach ($output as $code) {
        $this->assertEquals(8, strlen($code), "Code is of the proper length for $mol_file.");
      }
    }
    catch (Exception $e) {
    }
  }
}
