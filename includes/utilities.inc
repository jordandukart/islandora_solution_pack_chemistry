<?php

/**
 * @file
 * Holds helper functions for the Islandora Chemistry Solution Pack.
 */

/**
 * Get a select drop down form element for selecting a chemistry MIME type.
 *
 * @return array
 *   A form select element for chemistry MIME types.
 */
function islandora_chemistry_mime_selector() {
  return array(
    '#title' => t('File Type'),
    '#description' => t('Select the file type for uploaded file(s).'),
    '#type' => 'select',
    '#options' => islandora_chemistry_get_chemistry_mimes(),
  );
}

/**
 * Get chemistry MIME types.
 *
 * @return array
 *   An array with MIME types as keys and human readable names as values.
 */
function islandora_chemistry_get_chemistry_mimes() {
  return array(
    'chemical/x-mdl-molfile' => t('MDL Molfile'),
    'chemical/x-xyz' => t('XYZ format'),
    'chemical/x-pdb' => t('PDB'),
    'chemical/x-cdx' => t('ChemDraw CDX'),
    'chemical/x-chem3d' => t('ChemDraw 3D'),
    'chemical/x-chemdraw' => t('ChemDraw file'),
    'chemical/x-cif' => t('Crystallographic Information File'),
    'chemical/x-cml' => t('Chemical Markup Language'),
    'chemical/x-gamess-input' => t('GAMESS Input'),
    'chemical/x-gamess-output' => t('GAMESS Output'),
    'chemical/x-gaussian-cube' => t('Gaussian Cube'),
    'chemical/x-gaussian-input' => t('Gaussian 98/03 Cartesian Input'),
    'chemical/x-gaussian-log' => t('Gaussian 98/03 Output'),
    'chemical/x-jcamp-dx' => t('JCAMP Spectroscopic Data Exchange Format or OpenDX Grid'),
    'chemical/x-mopac-input' => t('MOPAC Cartesian'),
  );
}

/**
 * Retrieves RDF from ChemSpider given an InChI key.
 *
 * @param string $inchi_key
 *   A string representing an InChI key.
 *
 * @return array
 *   An array detailing the outcome of the http request containing:
 *   -success: A bool denoting whether retrieving the RDF was successful.
 *   -data: A string with the data returned from the request.
 *   -error: An string denoting an error that occured with the rquest.
 */
function islandora_chemistry_get_rdf_from_chemspider($inchi_key) {
  $chemspider_url = "http://rdf.chemspider.com";
  $options = array(
    'headers' => array(
      'Accept' => 'application/rdf+xml',
    ),
  );
  $response = drupal_http_request("{$chemspider_url}/{$inchi_key}", $options);
  // ChemSpider gives back relative redirect paths which breaks
  // drupal_http_request: https://drupal.org/node/164365.
  if (isset($response->redirect_code)) {
    $redirect_response = drupal_http_request("{$chemspider_url}{$response->redirect_url}", $options);
    if ($redirect_response->code == '200') {
      // To save us for doing order dependant XPATHs let's construct the
      // ChemSpider URL now.
      $chemspider_rdf_url = "http://www.chemspider.com{$response->redirect_url}";
      $chemspider_url = str_replace('rdf', 'html', $chemspider_rdf_url);
      return array(
        'success' => TRUE,
        'data' => $redirect_response->data,
        'chemspider_url' => $chemspider_url,
      );
    }
    else {
      return array(
        'success' => FALSE,
        'data' => $response->data,
        'error' => $response->error,
        'code' => $response->code,
      );
    }
  }
  else {
    return array(
      'success' => FALSE,
      'data' => $response->data,
      'error' => $response->error,
      'code' => $response->code,
    );
  }
}

/**
 * Searches for an IUPAC name and ChemSpider URL from an InChI key.
 *
 * @param string $inchi_key
 *   A string representing an InChI key.
 *
 * @return array
 *   An array containing:
 *   - iupac_name: A string denoting the IUPAC name or a message detailing
 *     the operation failing.
 *   - synonyms: An array of synonyms, if the request was successful.
 *   - chemspider_url: A string denoting the ChemSpider URL for the structure
 *     matching the InChI key, or FALSE if the object didn't exist.
 */
function islandora_chemistry_get_data_from_chemspider($inchi_key) {
  $rdf_response = islandora_chemistry_get_rdf_from_chemspider($inchi_key);
  if ($rdf_response['success']) {
    // Need to parse the IUPAC name out of RDF.
    $rdf_xml = simplexml_load_string($rdf_response['data']);
    $rdf_xml->registerXPathNamespace('chemdomain', 'http://www.polymerinformatics.com/ChemAxiom/ChemDomain.owl#');
    $iupac_xpath = $rdf_xml->xpath('chemdomain:Synonym/chemdomain:hasValue/text()');
    if ($iupac_xpath) {
      $iupac_name = (string) reset($iupac_xpath);
    }
    else {
      $iupac_name = t('No synonyms available');
    }
    return array(
      'iupac_name' => $iupac_name,
      'synonyms' => array_map('strval', $iupac_xpath),
      'chemspider_url' => $rdf_response['chemspider_url'],
    );
  }
  else {
    return array(
      'iupac_name' => t('Structure not found.'),
      'chemspider_url' => FALSE,
    );
  }
}

/**
 * Gets the InChIKey from a given MOL.
 *
 * @param string $mol
 *   String representation of a molecule.
 *
 * @return string
 *   The InChIKey for the molecule.
 */
function islandora_chemistry_get_inchikey_from_mol($mol) {
  module_load_include('inc', 'islandora_chemistry', 'includes/commands/openbabel');
  $options = new \Islandora\Chemistry\OpenBabel\Options(array(
    'i' => 'mol',
    'o' => 'inchi',
    'x' => 'K,',
  ));
  $options->setStdIn($mol);
  $result = \Islandora\Chemistry\OpenBabel\execute(
    FALSE,
    $options
  );
  return $result;
}

/**
 * Get synonyms for the given Inchi key(s).
 *
 * @param string|array $inchi_key
 *   Either a string or and array of strings to look up.
 *
 * @return array
 *   An associative array mapping the "classification" to an array containing
 *   the synonym values.
 */
function islandora_chemistry_get_synonyms_from_cactus($inchi_key) {
  $inchi_keys = is_array($inchi_key) ? $inchi_key : array($inchi_key);

  $map_key_to_results = function ($key) {
    $doc = new DOMDocument();
    $synonyms = array();
    $cactus_response = drupal_http_request(
      "http://cactus.nci.nih.gov/chemical/structure/$key/names/xml",
      array('timeout' => 5)
    );
    if ($cactus_response->code != 200) {
      return $synonyms;
    }
    $doc->loadXML($cactus_response->data);
    $xpath = new DOMXPath($doc);
    foreach ($xpath->query('/request/data/item') as $item) {
      $synonyms[$item->getAttribute('classification')][] = (string) $item->nodeValue;
    }
    return $synonyms;
  };

  $mapped_synonyms = array_map($map_key_to_results, $inchi_keys);
  $merged_synonyms = empty($mapped_synonyms) ?
    array() :
    call_user_func_array('array_merge_recursive', $mapped_synonyms);
  $unique_synonyms = array_map('array_unique', $merged_synonyms);
  return $unique_synonyms;
}

/**
 * Map output of obprop into arrays.
 *
 * @param string $file_uri
 *   A file URI of the file to process.
 *
 * @return array
 *   A numerically index array of associative arrays, each containing:
 *   - name
 *   - formula
 *   - mol_weight
 *   - exact_mass
 *   - canonical_SMILES
 *   - InChI
 *   - num_atoms
 *   - num_bonds
 *   - num_residues
 *   - sequence
 *   - num_rings
 *   - logP
 *   - PSA
 *   - MR
 */
function islandora_chemistry_get_chem_properties($file_uri) {
  $desc_spec = array(
    0 => array('pipe', 'r'),
    1 => array('pipe', 'w'),
    2 => array('pipe', 'w'),
  );
  $pipes = array();
  $command = format_string('!command !file', array(
    '!command' => escapeshellarg(variable_get('islandora_chemistry_obprop', '/usr/bin/obprop')),
    '!file' => escapeshellarg(drupal_realpath($file_uri)),
  ));
  $proc = proc_open($command, $desc_spec, $pipes);
  // Don't really need stdin...
  fclose($pipes[0]);
  // XXX: Some files generate a large amount of stderr output, which somehow
  // will block the read from stdout (due to filling buffers, or something of
  // the like?). Closing stderr stops it from blocking.
  fclose($pipes[2]);
  $output = stream_get_contents($pipes[1]);
  fclose($pipes[1]);

  $molecules = array_filter(array_map('trim', explode('$$$$', $output)));
  $map_molecules = function ($text) {
    // XXX: obprop gives a tab-separated-esque output with two columns:
    // - The first column indicates the "field"
    // - The second column contains the value for the given field.
    preg_match_all('/([^ \n\t]+)\s+([^\n\t]+)/', $text, $matches);
    return array_combine($matches[1], $matches[2]);
  };

  $raw_output = array_map($map_molecules, $molecules);

  // XXX: obprop will give a value of "-" (a hyphen) when it is unable to
  // provide a value (an InChI value if there are more the 1000 atoms, for
  // example). Let's filter these values out.
  $no_value_filter = function ($field_value) {
    return $field_value != '-';
  };
  $filtered_output = array();
  foreach ($raw_output as $molecule) {
    $filtered_output[] = array_filter($molecule, $no_value_filter);
  }

  return $filtered_output;
}
