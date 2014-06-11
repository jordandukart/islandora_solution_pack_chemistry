<?php

/**
 * @file
 * Helper functions for Checkmol/cmmmsrv.
 */

/**
 * Identify functional fragments in a MOL datastream.
 *
 * @param AbstractObject $object
 *   The object for which to extract functional fragments.
 *
 * @return array
 *   An array of strings as returned by
 *   \Islandora\Chemistry\Checkmol\get8DigitCodes().
 */
function islandora_chemistry_checkmol_get_fragments(AbstractObject $object) {
  $mol_uri = file_create_filename('fragments.mol', 'temporary://');

  // XXX: If we stop generating the MOL datastream, we'd have to coerce the OBJ
  // into a MOL file, to feed it off to checkmol...
  $object['MOL']->getContent($mol_uri);
  module_load_include('inc', 'islandora', 'includes/utilities');
  $mol_file = islandora_temp_file_entry($mol_uri);

  module_load_include('inc', 'islandora_chemistry', 'includes/commands/checkmol');
  try {
    $checkmol = new \Islandora\Chemistry\Checkmol(
      variable_get('islandora_chemistry_cmmmsrv_host', 'localhost'),
      variable_get('islandora_chemistry_cmmmsrv_port', 55624)
    );
    $to_return = $checkmol->get8DigitCodes($mol_file->uri);
    file_delete($mol_file);
  }
  catch (Exception $e) {
    file_delete($mol_file);
    throw $e;
  }
  return $to_return;
}
