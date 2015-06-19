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

/**
 * Derivative callback for generating the CML.
 */
function islandora_chemistry_derive_cml(AbstractObject $object, $force = FALSE) {
  // We need the MOL eventually... Abort if it's not there...
  if ((isset($object['CML']) && !$force) || !isset($object['MOL'])) {
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
    $file_in = islandora_chemistry_derive_download_as_temp($object['OBJ']);
    $java = variable_get('islandora_chemistry_java', 'java');
    $jar = drupal_realpath(libraries_get_path('jumbo-converter') . '/retrievium-importer.jar');
    $cml_dir = drupal_realpath(file_default_scheme() . '://');
    $file_in_path = drupal_realpath($file_in->uri);
    $cml_base = pathinfo($file_in_path, PATHINFO_FILENAME);
    $cml_path = "$cml_dir/$cml_base.cml";
    exec("$java -jar $jar -o $cml_dir $file_in_path");
    file_delete($file_in);
    if (is_file($cml_path)) {
      $cml_string = file_get_contents($cml_path);
      file_unmanaged_delete($cml_path);
    }
    // Fallback to Open Bable if Jumbo fails.
    if (!isset($cml_string)) {
      module_load_include('inc', 'islandora_chemistry', 'includes/derive.cml');
      $cml_string = islandora_chemistry_derive_string_from_openbabel($object, 'OBJ', array(
        'o' => 'cml',
      ));
    }
    $cml_doc = new DOMDocument();
    $cml_doc->formatOutput = TRUE;
    $cml_doc->preserveWhiteSpace = FALSE;
    $cml_doc->loadXML($cml_string);
    islandora_chemistry_derive_cml_add_fields($object, $cml_doc);

    $datastream = isset($object['CML']) ?
      $object['CML'] :
      $object->constructDatastream('CML', 'M');

    if ($datastream->mimetype != 'chemical/x-cml') {
      $datastream->mimetype = 'chemical/x-cml';
    }
    if ($datastream->label != 'CML Datastream') {
      $datastream->label = 'CML Datastream';
    }
    $datastream->content = $cml_doc->saveXML();

    if (!isset($object['CML'])) {
      $object->ingestDatastream($datastream);
    }

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
 * Add fields to the CML document.
 *
 * @param AbstractObject $object
 *   The object for which CML is being built.
 * @param DOMDocument $cml_doc
 *   A DOMDocument containing the parsed base to which to add.
 */
function islandora_chemistry_derive_cml_add_fields(AbstractObject $object, DOMDocument $cml_doc) {
  module_load_include('inc', 'islandora_chemistry', 'includes/derive.cml');
  $cml_ns = 'http://www.xml-cml.org/schema';
  $mods_ns = 'http://www.loc.gov/mods/v3';
  $xpath = new DOMXPath($cml_doc);
  $xpath->registerNamespace('cml', $cml_ns);

  // Add all of our values into appropriate locations in the CML.
  $info = islandora_chemistry_derive_cml_get_info($object);
  $info = array_filter($info);

  // Little helper function for dealing with existing elements.
  $empty_element = function (DOMElement $element) {
    array_map(array($element, 'removeChild'), iterator_to_array($element->childNode));
  };
  // Ensure an entry for the CML schema is available.
  $ensure_schema_def = function (DOMElement $element, $namespace, $location) {
    $schema_loc = $element->getAttributeNS('http://www.w3.org/2001/XMLSchema-instance', 'schemaLocation');
    $schema_info = array_filter(preg_split('/\s+/', $schema_loc));
    foreach (array_chunk($schema_info, 2) as $schema_def) {
      list($ns_uri, $schema_uri) = $schema_def;
      if ($ns_uri == $namespace) {
        return;
      }
    }
    $schema_info[] = $namespace;
    $schema_info[] = $location;
    $element->setAttributeNS('http://www.w3.org/2001/XMLSchema-instance', 'xsi:schemaLocation', implode(' ', $schema_info));
  };

  $cml_root = $cml_doc->documentElement;
  $ensure_schema_def($cml_root, $cml_ns, file_create_url(drupal_get_path('module', 'islandora_chemistry') . '/xml/cml3-0_schema.xsd'));
  $ensure_schema_def($cml_root, $mods_ns, 'http://www.loc.gov/standards/mods/v3/mods-3-5.xsd');

  $mods_list = $xpath->query('cml:metadataList[@name="mods:mods" and @convention="islandora:sp_chem_CM"]', $cml_root);
  if ($mods_list->length === 0) {
    $mods_parent = $cml_doc->createElementNS($cml_ns, 'metadataList');
    $mods_parent->setAttribute('convention', 'islandora:sp_chem_CM');
    $cml_root->appendChild($mods_parent);
  }
  else {
    $mods_parent = $mods_list->item(0);
    $empty_element($mods_parent);
  }

  $mods_root = $cml_doc->createElementNS($mods_ns, 'mods');
  $mods_parent->appendChild($mods_root);

  if (isset($info['inchikey'])) {
    foreach ($info['inchikey'] as $key) {
      $inchikey_element = $cml_doc->createElementNS($mods_ns, 'identifier', $key);
      $inchikey_element->setAttribute('type', 'inchikey');
      $mods_root->appendChild($inchikey_element);
    }
  }
  if (isset($info['title'])) {
    $title_info_element = $cml_doc->createElementNS($mods_ns, 'titleInfo');
    $title_info_element->setAttribute('type', 'uniform');
    $title_info_element->setAttribute('authority', 'IUPAC:trivial');
    $title_element = $cml_doc->createElementNS($mods_ns, 'title', $info['title']);
    $title_info_element->appendChild($title_element);
    $mods_root->appendChild($title_info_element);
  }
  if (isset($info['synonyms'])) {
    foreach ($info['synonyms'] as $source => $values) {
      foreach ($values as $synonym) {
        $ti = $cml_doc->createElementNS($mods_ns, 'titleInfo');
        $ti->setAttribute('type', 'alternative');
        $ti->setAttribute('authority', $source);
        $title = $cml_doc->createElementNS($mods_ns, 'title', $synonym);
        $ti->appendChild($title);
        $mods_root->appendChild($ti);
      }
    }
  }

  $list = array(
    'formula' => 'formula',
    'mweight' => 'molecular weight',
    'inchi' => 'inchi',
    'fragments' => 'functional fragment',
  );
  $list_intersection = array_intersect_key($list, $info);
  if ($list_intersection) {
    foreach ($list_intersection as $source => $type) {
      foreach ((array) $info[$source] as $value) {
        $note = $cml_doc->createElementNS($mods_ns, 'note', $value);
        $note->setAttribute('type', $type);
        $mods_root->appendChild($note);
      }
    }
  }
  $cml_doc->normalizeDocument();
}

/**
 * Derivative callback for fingerprinting.
 */
function islandora_chemistry_derive_fingerprint(AbstractObject $object, $force = FALSE) {
  if (!isset($object['MOL'])) {
    return;
  }

  $to_return = array(
    'success' => TRUE,
    'messages' => array(),
  );
  $subs = array(
    '@id' => $object->id,
  );

  try {
    module_load_include('inc', 'islandora_chemistry', 'includes/fingerprint');
    module_load_include('inc', 'islandora_chemistry', 'includes/utilities');
    islandora_chemistry_fingerprint(
      islandora_chemistry_get_inchikey_from_mol($object['MOL']->content),
      $object['MOL']->content
    );

    $to_return['messages'][] = array(
      'message' => t('Generated and added fingerprint to the database for @id.'),
      'message_sub' => $subs,
      'type' => 'dsm',
    );
  }
  catch (Exception $e) {
    $to_return['success'] = FALSE;
    $to_return['messages'][] = array(
      'message' => t('Failed when generating the fingerprint for @id.'),
      'message_sub' => $subs,
      'type' => 'dsm',
    );
  }
  return $to_return;
}

/**
 * Sets the label of the object based on the OBJ's filename.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing an object within Fedora.
 *
 * @param bool $force
 *   Whether derivation creation is being forced or not.
 */
function islandora_chemistry_set_object_label(AbstractObject $object, $force = FALSE) {
  $object->label = $object['OBJ']->label;
}
