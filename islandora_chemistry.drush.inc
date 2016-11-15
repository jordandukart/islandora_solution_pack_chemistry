<?php
/**
 * @file
 * Implementation of Drush hooks.
 */

/**
 * Implements hook_drush_command().
 */
function islandora_chemistry_drush_command() {
  $items = array();
  $items['islandora_chemistry_create_mol'] = array(
    'aliases' => array('iccm'),
    'drupal dependencies' => array(
      'islandora',
      'islandora_chemistry',
    ),
    'options' => array(
      'filepath' => array(
        'description' => 'Path to file to be processed.',
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );
  $items['islandora_chemistry_create_frags'] = array(
    'aliases' => array('iccf'),
    'drupal dependencies' => array(
      'islandora',
      'islandora_chemistry',
    ),
    'options' => array(
      'filepath' => array(
        'description' => 'Path to file to be processed.',
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );
  return $items;
}

/**
 * Creates a MOL file from an input file.
 */
function drush_islandora_chemistry_create_mol() {
  $file_path = drush_get_option('filepath');
  islandora_chemistry_create_mol_file($file_path);
}

function islandora_chemistry_create_mol_file($file_path) {
  // Creates a MOL file.
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora_chemistry', 'includes/commands/openbabel');
  $input_options = array(
    'o' => 'mol',
  );
  $fileinfo = pathinfo($file_path);
  $filename = "{$fileinfo['filename']}-MOL";
  file_create_filename($filename, 'temporary://');
  $options = new Islandora\Chemistry\OpenBabel\Options(array(
      'O' => drupal_realpath("temporary://$filename"),
    ) + $input_options);
  $result = Islandora\Chemistry\OpenBabel\execute(
    drupal_realpath($file_path),
    $options
  );
  $full_path = drupal_realpath("temporary://$filename");
  if (!file_exists($full_path) || filesize($full_path) === 0) {
    drush_log(dt("$file_path failed horribly."));
  }
}

/**
 * Creates fragments from an input file.
 */
function drush_islandora_chemistry_create_frags() {
  $file_path = drush_get_option('filepath');
  islandora_chemistry_create_fragments($file_path);
}

function islandora_chemistry_create_fragments($file_path) {
  module_load_include('inc', 'islandora_chemistry', 'includes/commands/checkmol');
  try {
    $checkmol = new \Islandora\Chemistry\Checkmol(
      variable_get('islandora_chemistry_cmmmsrv_host', 'localhost'),
      variable_get('islandora_chemistry_cmmmsrv_port', 55624)
    );
    $to_return = $checkmol->get8DigitCodes($file_path);
  }
  catch (Exception $e) {
    drush_log("$file_path failed horribly {$e->getMessage()}");
    throw $e;
  }
}
