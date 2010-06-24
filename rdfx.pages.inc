<?php
// $Id:  $

/**
 * Callback function for viewing an entity as RDF.
 */
function rdfx_entity_view($type, $entity) {
  // Constructs RDF representing the entity object.
  $function = 'rdfx_build_rdf_' . $type;
  if (function_exists($function)) {
    $rdf = $function($entity);
  }

  // Allow modules to make their own additions to the RDF of the entity.
  // Example: access control, etc.
  // @todo test
  // @todo drupal_alter()
  module_invoke_all('rdfx_entity_build', $entity);

  // Ouptuts the entity as RDF.
  // @todo allow to specify the default format in admin section.
  // @todo support format as argument in url and coneg.
  $ser_conf = array('ns' => rdfx_get_namespaces());
  $format = ARC2::getPreferredFormat('RDFXML');
  // Default to RDFXML since people are expecting some non-RDFa RDF at /rdf.
  // @todo push content negotation at the node/%node level.
  if ($format == 'HTML') {
    $format = 'RDFXML';
  }
  // Bug in ARC2 with chrome and safari, where XML is prefered format.
  if ($format == 'XML') {
    $format = 'RDFXML';
  }
  // Bug in ARC2 with curl, where NULL is prefered format.
  if (empty($format)) {
    $format = 'RDFXML';
  }
  $ser = ARC2::getSer($format, $ser_conf);
  //$ser = ARC2::getTurtleSerializer($ser_conf);
  drupal_add_http_header('Content-Type', $ser->content_header);
  print $ser->getSerializedIndex($rdf->index);
}

