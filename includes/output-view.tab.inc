<?php

/**
 * @file
 * Function(s) associated with the "Output" view.
 */

/**
 * Generate the datastream view with a download link.
 *
 * @param AbstractObject $object
 *   The object for which to generate the view.
 *
 * @return array
 *   A renderable array.
 */
function islandora_chemistry_datastream_view(AbstractObject $object, $dsid = 'OBJ') {
  $datastream = $object[$dsid];
  $output = array(
    array(
      '#type' => 'markup',
      '#theme' => 'islandora_datastream_download_link',
      '#datastream' => $datastream,
    ),
  );
  // MIME-Types of text-based file formats.
  // TODO: Create a more complete MIME-Type mapping, to avoid having arrays of
  // MIME-Types scattered through the code.
  $text_chemical_mimes = array(
    'chemical/x-cml',
    'chemical/x-mdl-molfile',
    'chemical/x-xyz',
    'chemical/x-pdb',
    'chemical/x-cdx',
    'chemical/x-cif',
    'chemical/x-gamess-input',
    'chemical/x-gamess-output',
    'chemical/x-gaussian-cube',
    'chemical/x-gaussian-input',
    'chemical/x-gaussian-log',
    'chemical/x-jcamp-dx',
    'chemical/x-mopac-input',
  );
  // Mapping of some text-based format to alternative MIME-type, to provided a
  // better-supported value to GeSHi.
  $alternative_mime_map = array(
    'chemical/x-cml' => 'application/xml',
  );
  $primary_type = substr($datastream->mimetype, 0, strpos($datastream->mimetype, '/'));
  if (in_array($datastream->mimetype, $text_chemical_mimes) || $primary_type == 'text') {
    $mimetype = isset($alternative_mime_map[$datastream->mimetype]) ?
      $alternative_mime_map[$datastream->mimetype] :
      $datastream->mimetype;
    require_once libraries_get_path('geshi') . '/geshi.php';
    $mime_detect = new MimeDetect();
    $geshi = new GeSHi();
    $geshi->enable_classes();
    // Override the default code style as it causes text-alignment issues.
    $geshi->set_code_style('', FALSE);
    $geshi->set_header_type(GESHI_HEADER_DIV);
    $geshi->enable_line_numbers(GESHI_FANCY_LINE_NUMBERS, 10);
    $geshi->set_overall_class('islandora-chemistry-datastream-view');
    $geshi->set_source($datastream->content);
    $geshi->set_language($geshi->get_language_name_from_extension($mime_detect->getExtension($mimetype)));
    $output[] = array(
      '#type' => 'markup',
      '#attached' => array(
        'css' => array(
          array(
            'type' => 'inline',
            'data' => $geshi->get_stylesheet(),
          ),
        ),
      ),
      '#markup' => $geshi->parse_code(),
    );
  }

  return $output;
}
