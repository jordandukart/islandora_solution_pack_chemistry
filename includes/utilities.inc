<?php

/**
 * @file
 * Miscellaneous utility functions.
 */


/**
 * Set the title of the page to something more appropriate.
 *
 * This would be better done by setting the label during the ingest proper...
 * nowhere this is really able to be done, at the time of writing. (Would be
 * when generating the CML derivative).
 *
 * @param AbstractObject $object
 *   The object for which to set the title.
 */
function islandora_chemistry_set_title(AbstractObject $object) {
  $title = islandora_chemistry_get_title($object);
  if ($title) {
    drupal_set_title($title);
  }
}

/**
 * Get something to use as a title.
 *
 * Try to sniff something to use as a more usable title out of the CML.
 *
 * @param AbstractObject $object
 *   The object for which to sniff a title.
 *
 * @return string|bool
 *   Either a string containing the sniffed title, or FALSE if we didn't find
 *   anything.
 */
function islandora_chemistry_get_title(AbstractObject $object) {
  if (!(isset($object['CML']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['CML']))) {
    // Do not have CML to fetch from...
    return FALSE;
  }

  $cml = new SimpleXMLElement($object['CML']->content);
  $cml->registerXPathNamespace('cml', 'http://www.xml-cml.org/schema/cml2/core');

  // Attempt to use IUPAC:trivial name.
  $titles = $cml->xpath('//cml:molecule/cml:name[@convention="IUPAC:trivial"]');
  $title_element = reset($titles);
  if ($title_element) {
    $raw_title = (string) $title_element;
    $title = ucfirst(strtolower(ltrim($raw_title)));
    return $title;
  }

  // Fall-back to trying to use synonyms.
  $synonyms = $cml->xpath('//cml:molecule/cml:alternative[@type="synonym"]');
  $reducer = function ($carry, $current) {
    return array_merge($carry, explode(', ', (string) $current));
  };
  $synonym_strings = array_reduce($synonyms, $reducer, array());
  $synonym = reset($synonym_strings);
  if ($synonym) {
    return $synonym;
  }
}
