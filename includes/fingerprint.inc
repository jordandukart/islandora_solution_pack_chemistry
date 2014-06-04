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
 */
function islandora_chemistry_set_inchikey($columns) {
  return db_insert('islandora_chemistry_fingerprints')
  ->fields($columns)
  ->execute();
}

/**
 * Run fingerprint generation.
 *
 * @param string $filepath
 *   The path to the output temp file.
 *
 * @return int
 *   Fingerprint's return code.
 */
function islandora_chemistry_run_fingerprint($filepath) {
  $fingerprint_path = drupal_realpath(
    drupal_get_path('module', 'islandora_chemistry') . '/cli/fingerprint.py'
  );
  $python = variable_get('islandora_chemistry_python', '/usr/bin/python');
  $fingerprint_command = "$python $fingerprint_path $filepath";
  $fingerprint_process = proc_open($fingerprint_command);
  $fingerprint_exit_code = $proc_close($fingerprint_process);

  return $fingerprint_exit_code;
}
