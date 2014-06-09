<?php

/**
 * @file
 * This file contains derivative functions.
 */

/**
 * Derivative callback to generate MOL from the OBJ.
 */
function islandora_chemistry_derive_mol(AbstractObject $object, $force = FALSE, $hook = array()) {
  if (isset($object['MOL']) && !$force) {
    return;
  }

  $to_return = array(
    'success' => FALSE,
    'messages' => array(),
  );
  $subs = array(
    '@datastream' => 'MOL',
    '@id' => $object->id,
  );
  try {
    islandora_chemistry_derive_datastream_from_openbabel($object, 'OBJ', 'MOL', 'chemical/x-mdl-molfile', array(
      'o' => 'mol',
    ));
    $to_return['success'] = TRUE;
    $to_return['messages'][] = array(
      'message' => t('Generated and added @datastream derivative to @id.'),
      'message_sub' => $subs,
      'type' => 'dsm',
    );
  }
  catch (IslandoraChemistryDerivativeException $e) {
    $to_return['messages'][] = array(
      'message' => t('Failed when generating @datastream to add to @id.'),
      'message_sub' => $subs,
      'type' => 'dsm',
    );
  }

  return $to_return;
}

/**
 * Derivative callback to generate TN from the OBJ.
 */
function islandora_chemistry_derive_tn(AbstractObject $object, $force = FALSE, $hook = array()) {
  return islandora_chemistry_derive_image($object, 'TN', 100, $force, $hook);
}

/**
 * Derivative callback to generate MEDIUM from the OBJ.
 */
function islandora_chemistry_derive_medium(AbstractObject $object, $force = FALSE, $hook = array()) {
  return islandora_chemistry_derive_image($object, 'MEDIUM', 250, $force, $hook);
}

/**
 * Image derivative helper; generate the JPEG and add it to the object.
 */
function islandora_chemistry_derive_image(AbstractObject $object, $dest, $width, $force = FALSE, $hook = array()) {
  if (isset($object[$dest]) && !$force) {
    return;
  }

  $to_return = array(
    'success' => FALSE,
    'messages' => array(),
  );
  $subs = array(
    '@datastream' => $dest,
    '@id' => $object->id,
  );
  try {
    $output_file = islandora_chemistry_derive_jpeg_from_svg($object, $width);
    islandora_chemistry_derive_add_datastream($object, $dest, $output_file);
    $to_return['success'] = TRUE;
    $to_return['messages'][] = array(
      'message' => t('Generated and added @datastream derivative to @id.'),
      'message_sub' => $subs,
      'type' => 'dsm',
    );
  }
  catch (IslandoraChemistryDerivativeException $e) {
    $to_return['messages'][] = array(
      'message' => t('Failed when generating @datastream to add to @id.'),
      'message_sub' => $subs,
      'type' => 'dsm',
    );
  }
  if (isset($output_file)) {
    file_delete($output_file);
  }
  return $to_return;
}

/**
 * Image derivative helper; actually generate a JPEG from the OBJ.
 */
function islandora_chemistry_derive_jpeg_from_svg(AbstractObject $object, $width = 100) {
  $svg_file = islandora_chemistry_derive_file_from_openbabel($object, 'OBJ', 'image/svg+xml', array(
    // "png" doesn't work... Guess we'll convert 'em to jpegs...
    'o' => 'svg',
  ));
  $svg_file = file_move($svg_file, file_create_filename('temp.svg', 'temporary://'));

  // Convert to JPEG, based on the file extension.
  // XXX: Cannot use Drupal's image toolkit to to lack of SVG support.
  $output_uri = file_create_filename('temp.jpeg', 'temporary://');
  $args = array(
    "-resize {$width}x{$width}",
    '-gravity center',
    '-modulate 100,140',
    "-extent {$width}x{$width}",
  );
  $to_return = NULL;
  if (islandora_chemistry_imagemagick_convert($svg_file->uri, $output_uri, $args)) {
    $output_file = islandora_temp_file_entry($output_uri, 'image/jpeg');
    $to_return = $output_file;
  }
  file_delete($svg_file);
  return $to_return;
}

/**
 * OpenBabel derivative helper.
 *
 * Grab the indicated datastream and return the output based on the $input
 * options.
 *
 * @param AbstractObject $object
 *   The object to which the datastream belongs.
 * @param string $source
 *   The datastream ID of the datastream to grab.
 * @param string $dest_mime
 *   The mimetype of the file output.
 * @param array $input_options
 *   An array of options as accepted by \Islandora\Chemistry\OpenBabel\Options.
 *   We force the "O" (output file) parameter to be a file we work with.
 *
 * @return object
 *   A Drupal file-object, which has been populated with the results of the
 *   OpenBabel call.
 */
function islandora_chemistry_derive_file_from_openbabel(AbstractObject $object, $source, $dest_mime, $input_options) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora_chemistry', 'includes/commands/openbabel');

  $file = islandora_chemistry_derive_download_as_temp($object[$source]);
  $name = drupal_tempnam('temporary://', 'islandora_chemistry');

  $options = new Islandora\Chemistry\OpenBabel\Options(array(
    'O' => drupal_realpath($name),
  ) + $input_options);
  $result = Islandora\Chemistry\OpenBabel\execute(
    drupal_realpath($file->uri),
    $options
  );
  $output_file = islandora_temp_file_entry($name, $dest_mime);

  file_delete($file);
  if (!file_exists($output_file->uri) || filesize($output_file->uri) === 0) {
    throw new IslandoraChemistryDerivativeException('Failed to transform.');
  }
  return file_save($output_file);
}

/**
 * OpenBabel derivative helper.
 *
 * Grab the indicated datastream and return the output based on the $input
 * options.
 *
 * @param AbstractObject $object
 *   The object to which the datastream belongs.
 * @param string $source
 *   The datastream ID of the datastream to grab.
 * @param array $input_options
 *   An array of options as accepted by \Islandora\Chemistry\OpenBabel\Options.
 *
 * @return string
 *   A string, which has been populated with the results of the OpenBabel call.
 */
function islandora_chemistry_derive_string_from_openbabel(AbstractObject $object, $source, $input_options) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora_chemistry', 'includes/commands/openbabel');

  $file = islandora_chemistry_derive_download_as_temp($object[$source]);
  $name = drupal_tempnam('temporary://', 'islandora_chemistry');

  $options = new Islandora\Chemistry\OpenBabel\Options($input_options);
  $result = Islandora\Chemistry\OpenBabel\execute(
    drupal_realpath($file->uri),
    $options
  );
  file_delete($file);
  return $result;
}

/**
 * Derivative helper; Derive a file and add it.
 */
function islandora_chemistry_derive_datastream_from_openbabel(AbstractObject $object, $source, $dest, $dest_mime, $input_options) {
  $output_file = islandora_chemistry_derive_file_from_openbabel($object, $source, $dest_mime, $input_options);
  islandora_chemistry_derive_add_datastream($object, $dest, $output_file);
  file_delete($output_file);
}

/**
 * Derivative helper; add/update the given datastream.
 */
function islandora_chemistry_derive_add_datastream(AbstractObject $object, $dest, $output_file) {
  $datastream = isset($object[$dest]) ?
    $object[$dest] :
    $object->constructDatastream($dest, 'M');

  $label = "$dest Datastream";
  if (!isset($datastream->label) || $datastream->label != $label) {
    $datastream->label = $label;
  }
  if (!isset($datastream->mimetype) || $datastream->mimetype != $output_file->filemime) {
    $datastream->mimetype = $output_file->filemime;
  }
  $datastream->setContentFromFile($output_file->uri);

  if (!isset($object[$dest])) {
    $object->ingestDatastream($datastream);
  }
}

/**
 * Download a datastream.
 *
 * @param AbstractDatastream $datastream
 *   A datastream of which to download the latest version.
 *
 * @return object
 *   A Drupal file-object, describing where the file.
 */
function islandora_chemistry_derive_download_as_temp(AbstractDatastream $datastream) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $md = new MimeDetect();
  $name = file_create_filename("islandora_chemistry." . $md->getExtension($datastream->mimetype), 'temporary://');
  $datastream->getContent($name);
  $file = islandora_temp_file_entry($name, $datastream->mimetype);
  $file = file_save($file);
  return $file;
}

/**
 * Wrap convert calls...
 *
 * Copy/pasta from islandora_large_image.
 *
 * @see https://github.com/Islandora/islandora_solution_pack_large_image/blob/d5d32854062729eac0ab6e5c09bc45201256da31/includes/derivatives.inc#L320-L361
 */
function islandora_chemistry_imagemagick_convert($src, $dest, $args) {
  $src = drupal_realpath($src);
  $dest = drupal_realpath($dest);
  $context = array(
    'source' => $src,
    'destination' => $dest,
  );
  drupal_alter('imagemagick_arguments', $args, $context);

  // To make use of ImageMagick 6's parenthetical command grouping we need to
  // make the $source image the first parameter and $dest the last.
  // See http://www.imagemagick.org/Usage/basics/#cmdline
  $command = escapeshellarg($src) . ' ' . implode(' ', $args) . ' ' . escapeshellarg($dest);
  $output = array();
  $ret = -1;
  if (_imagemagick_convert_exec($command, $output, $ret) !== TRUE) {
    $variables = array(
      '@ret' => $ret,
      '@command' => $command,
      '!output' => implode('<br/>', $output),
    );
    watchdog('islandora_chemistry', 'ImageMagick failed to convert.<br/>Error: @ret<br/>Command: @command <br/>Output !output', $variables, WATCHDOG_ERROR);
    return FALSE;
  }
  return $dest;
}

class IslandoraChemistryDerivativeException extends Exception {
}

function islandora_chemistry_derive_cml(AbstractObject $object, $force = FALSE) {
  if (isset($object['CML']) && !$force) {
    return;
  }

  $to_return = array(
    'success' => FALSE,
    'messages' => array(),
  );
  $subs = array(
    '@datastream' => 'CML',
    '@id' => $object->id,
  );
  try {
    $cml_string = islandora_chemistry_derive_string_from_openbabel($object, 'OBJ', 'CML', array(
      'o' => 'cml',
    ));
    $cml_doc = new DOMDocument();
    $cml_doc->loadXML($cml_string);
    $to_return['success'] = TRUE;
    $to_return['messages'][] = array(
      'message' => t('Generated and added @datastream derivative to @id.'),
      'message_sub' => $subs,
      'type' => 'dsm',
    );
  }
  catch (IslandoraChemistryDerivativeException $e) {
    $to_return['messages'][] = array(
      'message' => t('Failed when generating @datastream to add to @id.'),
      'message_sub' => $subs,
      'type' => 'dsm',
    );
  }

  return $to_return;
}
