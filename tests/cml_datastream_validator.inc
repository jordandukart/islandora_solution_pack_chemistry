<?php
/**
 * @file
 * Asserts the validity of a MOL datastream.
 */

module_load_include('inc', 'islandora', 'tests/datastream_validators');

/**
 * A special DatastreamValidator specifically to verify MOL datastreams.
 */
class CMLDatastreamValidator extends DatastreamValidator {

  /**
   * Checks that the CML is XML.
   */
  public function assertIsXML() {
    $doc = new DOMDocument();
    $this->addResult($doc->loadXML($this->datastreamContent), 'CML stream was able to be parsed as XML.');
  }
}
