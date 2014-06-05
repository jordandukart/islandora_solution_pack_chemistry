<?php
/**
 * @file
 * Holds helper functions for fingerprint.py integration.
 */

/**
 * Create a fingerprint.
 *
 * @param array $columns
 *   Field array as key => value pairs, used in db_insert.
 *
 * @return int
 *   The id field for the created database entry.
 */
function islandora_chemistry_set_compound($columns) {
  return db_insert('islandora_chemistry_fingerprints')
    ->fields($columns)
    ->execute();
}

/**
 * Get a fingerprint, or verify it is set.
 *
 * @param string $inchikey
 *   The InChIKey to get the fingerprint for.
 *
 * @return mixed
 *   Associative array of database fields for the record.
 *   FALSE if there are is no existing entry for that InChIKey.
 */
function islandora_chemistry_get_compound($inchikey) {
  return db_select('islandora_chemistry_fingerprints')
    ->fields('islandora_chemistry_fingerprints')
    ->condition('inchikey', $inchikey)
    ->execute()
    ->fetchAssoc();
}

/**
 * Run fingerprint generation.
 *
 * @param string $mol_file_path
 *   The path to the .mol file to fingerprint.
 *
 * @return mixed
 *   Fingerprint's return code of not 0.
 *   The fingerprint as an array of fingerprint_$i => int if successful.
 */
function islandora_chemistry_run_fingerprint($mol_file_path) {
  $fingerprint_path = drupal_realpath(
    drupal_get_path('module', 'islandora_chemistry') . '/cli/fingerprint.py'
  );
  $python = variable_get('islandora_chemistry_python', '/usr/bin/python');
  $fingerprint_command = "$python $fingerprint_path $mol_file_path";
  $fingerprint_descriptorspec = array(
    0 => array('pipe', 'r'),
    1 => array('pipe', 'w'),
    2 => array('pipe', 'w'),
  );
  $fingerprint_pipes = array();
  $fingerprint_environment = array(
    'PYTHONPATH' => drupal_realpath(libraries_get_path('indigo-python')),
  );

  $fingerprint_process = proc_open(
    $fingerprint_command,
    $fingerprint_descriptorspec,
    $fingerprint_pipes,
    NULL,
    $fingerprint_environment
  );

  fclose($fingerprint_pipes[0]);
  $fingerprint_process_output = stream_get_contents($fingerprint_pipes[1]);
  fclose($fingerprint_pipes[1]);
  fclose($fingerprint_pipes[2]);
  $fingerprint_exit_code = proc_close($fingerprint_process);

  if ($fingerprint_exit_code) {
    return FALSE;
  }
  else {
    $numerical_fingerprint_array = explode(
      ', ',
      str_replace(
        array('[', ']', 'L'),
        array('', '', ''),
        $fingerprint_process_output
      )
    );
    foreach ($numerical_fingerprint_array as $index => $part) {
      $fingerprint["fingerprint_$index"] = $part;
    }
    return $fingerprint;
  }
}

/**
 * Gets or sets a fingerprint if necessary.
 *
 * @param string $inchikey
 *   The InChIKey to get the fingerprint for.
 *
 * @param string $mol_file_path
 *   The path to the .mol file to fingerprint.
 */
function islandora_chemistry_fingerprint($inchikey, $mol_file_path) {
  $fingerprint = islandora_chemistry_get_compound($inchikey);

  if (!$fingerprint) {
    $fingerprint = islandora_chemistry_run_fingerprint($mol_file_path);
    if ($fingerprint !== FALSE) {
      $fingerprint = $fingerprint + array('inchikey' => $inchikey);
      islandora_chemistry_set_compound($fingerprint);
    }
    else {
      return FALSE;
    }
  }

  if (isset($fingerprint['id'])) {
    unset($fingerprint['id']);
  }
  unset($fingerprint['inchikey']);
  return $fingerprint;
}
