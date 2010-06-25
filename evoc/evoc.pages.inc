<?php
// $Id:  $

/**
 * @file
 * Page callbacks for importing a vocabulary.
 */

/**
 * Present a node submission form or a set of links to such forms.
 */
function evoc_import() {
  return drupal_get_form('evoc_import_form');
}


function evoc_import_form($form_state) {
  $form['help'] = array(
    '#markup' => 'This form allows you to import external RDF vocabularies into your site.
    These can later be used by other modules such as <a href="http://drupal.org/project/rdf">RDF UI</a> or <a href="http://drupal.org/project/neologism">Neologism</a>. Popular vocabularies include:<br />
    dc : http://purl.org/dc/terms/<br />
    foaf : http://xmlns.com/foaf/0.1/<br />
    sioc : http://rdfs.org/sioc/ns#<br />
    skos : http://www.w3.org/2008/05/skos#<br />
    doap : http://usefulinc.com/ns/doap#<br />
    ',
  );
  $form['prefix'] = array(
    '#type' => 'textfield',
    '#title' => t('Prefix'),
    '#required' => TRUE,
    '#default_value' => isset($form_state['values']['prefix']) ? $form_state['values']['prefix'] : NULL,
    '#description' => "Choose a prefix for this vocabulary. Example: dc, foaf, sioc etc. This prefix will later be used in the system to refer to this vocabulary.",
  );
  $form['ns_uri'] = array(
    '#type' => 'textfield',
    '#title' => t('Vocabulary URI'),
    '#required' => TRUE,
    '#default_value' => isset($form_state['values']['ns_uri']) ? $form_state['values']['ns_uri'] : NULL,
    '#description' => "Enter the URI of the vocabulary to import. Make sure it finishes by either / or #.",
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Submit',
  );
  return $form;
}

function evoc_import_form_validate($form, &$form_state) {
  // Load the XML Namespace regular expression patterns.
  module_load_include('inc', 'rdfx');

  $prefix = $form_state['values']['prefix'];
  $ns_uri = $form_state['values']['ns_uri'];

  // Ensure the that namespace is a valid URI.
  if (!valid_url($ns_uri, $absolute = TRUE)) {
    form_set_error('ns_uri', t('The Vocabulary URI must be a valid URI.'));
  }
  // Ensure the namespace URI ends in either a / or a #.
  if (!preg_match('/(\/|\#)$/', $ns_uri)) {
    form_set_error('ns_uri', t('The Vocabulary URI must end in either a / or a #.'));
  }
  // Ensure the prefix is well formed according to the specification.
  if (!preg_match('/^' . PREFIX .'$/', $prefix)) {
    form_set_error('prefix', t('The prefix must follow the !link.', array('!link' => '<a href="http://www.w3.org/TR/xml-names11/#NT-NCName">XML Namespace Specification</a>')));
  }
}

function evoc_import_form_submit($form, &$form_state) {
// @todo is that still the best way to keep the value when the form is reloaded?
  $form_state['storage']['values'] = $form_state['values'];
  $form_state['rebuild'] = true;

  evoc_import_vocabulary($form_state['values']['ns_uri'], $form_state['values']['prefix']);

  drupal_set_message(t('Your form has been saved...'));
}
