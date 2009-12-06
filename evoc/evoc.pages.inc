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
    These can later be used by other modules such as <a href="http://drupal.org/project/rdfcck">RDF CCK</a> or <a href="http://drupal.org/project/neologism">Neologism</a>. Popular vocabularies include:<br />
    dc : http://purl.org/dc/elements/1.1/<br />
    dcterms : http://purl.org/dc/terms/<br />
    dcmitype : http://purl.org/dc/dcmitype/<br />
    foaf : http://xmlns.com/foaf/0.1/<br />
    sioc : http://rdfs.org/sioc/ns#<br />
    skos : http://www.w3.org/2008/05/skos#<br />
    doap : http://usefulinc.com/ns/doap#<br />
    ',
  );
  $form['ns_uri'] = array(
    '#type' => 'textfield',
    '#title' => t('Vocabulary URI'),
    '#required' => TRUE,
    '#default_value' => isset($form_state['values']['ns_uri']) ? $form_state['values']['ns_uri'] : NULL,
    '#description' => "Enter the URI of the vocabulary to import. Make sure it finishes by either / or #.",
  );
  $form['prefix'] = array(
    '#type' => 'textfield',
    '#title' => t('Prefix'),
    '#required' => TRUE,
    '#default_value' => isset($form_state['values']['prefix']) ? $form_state['values']['prefix'] : NULL,
    '#description' => "Choose a prefix for this vocabulary. Example: dc, foaf, sioc etc. This prefix will later be used in the system to refer to this vocabulary.",
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Submit',
  );
  return $form;
}

function evoc_import_form_validate($form, &$form_state) {
  // Checks the prefix only contains letters.
  // @todo
}

function evoc_import_form_submit($form, &$form_state) {
  // @todo
  drupal_set_message(t('Your form has been saved.'));
}