<?php

/**
 * @file
 * Functions pertaining to the generation of the CML datastream in particular.
 */

/**
 * @return array
 *   An associative array containing:
 *   - inchikey: The inchikey of the molecule.
 *   - synonyms: An array of synonyms.
 *   - title: The name of the molecule.
 *   - fragments: An array of fragments (Checkmol).
 *   - mweight: The molecular weight.
 */
function islandora_chemistry_derive_cml_get_info(AbstractObject $object) {
  $to_return = array();
  $functions = array(
    'islandora_chemistry_derive_cml_get_inchikey',
    'islandora_chemistry_derive_cml_get_synonyms',
    'islandora_chemistry_derive_cml_get_title',
    'islandora_chemistry_derive_cml_get_fragments',
  );
  $args = array(&$to_return, $object);

  foreach ($functions as $function) {
    call_user_func_array($function, $args);
  }

  return $to_return;
}

function islandora_chemistry_derive_cml_get_inchikey(array &$info, AbstractObject $object) {
  if (isset($info['inchikey'])) {
    return;
  }

  module_load_include('inc', 'islandora_chemistry', 'includes/derivatives');
  $info += array(
    'inchikey' => islandora_chemistry_derive_string_from_openbabel($object, 'MOL', array(
      'i' => 'mol',
      'separate' => TRUE,
      'o' => 'inchi',
      'x' => 'K',
    )),
  );
}

function islandora_chemistry_derive_cml_get_synonyms(array &$info, AbstractObject $object) {
  if (isset($info['synonyms'])) {
    return;
  }

  islandora_chemistry_derive_cml_get_inchikey($info, $object);

  module_load_include('inc', 'islandora_chemistry', 'includes/utilities');
  $cs = islandora_chemistry_get_data_from_chemspider($info['inchikey']);
  $info['synonyms']['chem_spider'] = isset($cs['synonyms']) ?
    $cs['synonyms'] :
    array();

  $info['synonyms'] = array_merge($info['synonyms'], islandora_chemistry_get_synonyms_from_cactus($info['inchikey']));
}

function islandora_chemistry_derive_cml_get_title(array &$info, AbstractObject $object) {
  if (isset($info['title'])) {
    return;
  }

  islandora_chemistry_derive_cml_get_synonyms($info, $object);
  $sources = array_filter($info['synonyms']);
  $source = reset($sources);
  $info['title'] = $source ? reset($source) : FALSE;
}

function islandora_chemistry_derive_cml_get_fragments(array &$info, AbstractObject $object) {
  if (isset($info['fragments'])) {
    return;
  }

  module_load_include('inc', 'islandora_chemistry', 'includes/checkmol');
  $info['fragments'] = islandora_chemistry_checkmol_get_fragments($object);
}

function islandora_chemistry_derive_cml_get_properties($info, $object) {
  $map = array(
    'mol_weight' => 'weight',
    'formula' => 'formula',
    'InChI' => 'inchi',
  );
  if (empty(array_diff_key(array_flip($map), $info))) {
    return;
  }

  module_load_include('inc', 'islandora_chemistry', 'includes/utilties');
  module_load_include('inc', 'islandora_chemistry', 'includes/derivatives');
  $file =  islandora_chemistry_derive_download_as_temp($object['OBJ']);
  $properties = islandora_chemistry_get_chem_properties($file->uri);
  file_delete($file);

  if ($properties) {
    $mol = reset($properties);
    foreach ($map as $origin => $dest) {
      $info[$dest] = $mol[$origin];
    }
  }
}
