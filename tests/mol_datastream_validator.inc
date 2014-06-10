<?php
/**
 * @file
 * Asserts the validity of a MOL datastream.
 */

require_once __DIR__ . '/../../islandora/tests/datastream_validators.inc';

/**
 * A special DatastreamValidator specifically to verify MOL datastreams.
 */
class MOLDatastreamValidator extends DatastreamValidator {

  /**
   * Checks that the MOL codes are 8 digits in length.
   */
  public function assertMOL8DigitCodeString() {
    require_once __DIR__ . "/../includes/commands/checkmol.inc";
    $checkmol = new Islandora\Chemistry\Checkmol();
    try {
      // Get codes from the datastream content; check if they're all 8 digits.
      $output = $checkmol->get8DigitCodes($this->datastreamContent);
      foreach ($output as $key => $code) {
        $code_length = strlen($code);
        $index = $key + 1;
        $this->addResult(strlen($code) == 8, "MOL code $index of {$this->object->id} {$this->datastream} datastream contains $code_length characters (8 are expected).");
      }
    }
    // If something goes catastrophically wrong, make a fuss.
    catch (Exception $e) {
      $this->addResult(FALSE, "Exception encountered while attempting to get 8-digit codes from {$this->datastream} datastream of {$this->object->id}.");
    }
  }
}
