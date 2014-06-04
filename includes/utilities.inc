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
    'chemical/x-mdl-molfile' => 'MDL Molfile',
    'chemical/x-xyz' => 'XYZ format',
    'chemical/x-pdb' => 'PDB',
    'chemical/x-cdx' => 'ChemDraw CDX',
    'chemical/x-chem3d' => 'ChemDraw 3D',
    'chemical/x-chemdraw' => 'ChemDraw file',
    'chemical/x-cif' => 'Crystallographic Information File',
    'chemical/x-cml' => 'Chemical Markup Language',
    'chemical/x-gamess-input' => 'GAMESS Input',
    'chemical/x-gamess-output' => 'GAMESS Output',
    'chemical/x-gaussian-cube' => 'Gaussian Cube',
    'chemical/x-gaussian-input' => 'Gaussian 98/03 Cartesian Input',
    'chemical/x-gaussian-log' => 'Gaussian 98/03 Output',
    'chemical/x-jcamp-dx' => 'JCAMP Spectroscopic Data Exchange Format or OpenDX Grid',
    'chemical/x-mopac-input' => 'MOPAC Cartesian',
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
      'code' => '404',
    );
  }
}

/**
 * Searches for an IUPAC name and ChemSpider URL for an InChI key.
 *
 * @param string $inchi_key
 *   A string representing an InChI key.
 *
 * @return array
 *   An array containing:
 *   -iupac_name: A string denoting the IUPAC name or a message detailing
 *   the operation failing.
 *   -chemspider_url: A string denoting the ChemSpider URL for the structure
 *   matching the InChI key, or FALSE if the object didn't exist.
 */
function islandora_chemistry_get_data_from_chemspider($inchi_key) {
  $rdf_response = islandora_chemistry_get_rdf_from_chemspider($inchi_key);
  if ($rdf_response['success']) {
    // Need to parse the IUPAC name out of RDF.
    $rdf_xml = simplexml_load_string($rdf_response['data']);
    $iupac_xpath = $rdf_xml->xpath('chemdomain:Synonym/chemdomain:hasValue/text()');
    if ($iupac_xpath) {
      $iupac_name = (string) reset($iupac_xpath);
    }
    else {
      $iupac_name = t('No synonyms available');
    }
    return array(
      'iupac_name' => $iupac_name,
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
