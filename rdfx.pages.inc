<?php
// $Id:  $

/**
 * Callback function for viewing an entity as RDF.
 */
function rdfx_entity_view($type, $entity) {
  // Constructs RDF representing the entity object.
  $function = 'rdfx_build_' . $type . '_rdf';
  if (function_exists($function)) {
    $rdf = $function($entity);
  }

  // Allow modules to make their own additions to the RDF of the entity.
  // Example: access control, etc.
  module_invoke_all('rdfx_entity_build', $entity);

  // Ouptuts the entity as RDF.
  // @todo allow to specify the default format in admin section.
  // @todo support format as argument in url and coneg.
  $ser_conf = array('ns' => rdfx_get_namespaces());
  $ser = ARC2::getRDFXMLSerializer($ser_conf);
  //$ser = ARC2::getTurtleSerializer($ser_conf);
  drupal_add_http_header('Content-Type', $ser->content_header);
  print $ser->getSerializedIndex($rdf->index);
}

/**
 * Constructs an RDF representation of a node.
 */
function rdfx_build_node_rdf($node) {
  // URI of the node.
  $doc_uri = url('node/' . $node->nid, array('absolute' => TRUE));
  // URI of the resource being described in the node.
  // @todo allow for customized resource uris.
  $res_uri = url('node/' . $node->nid . '#this');

  // Instanciate node resource as ARC2 class and set base and namespaces.
  $res = ARC2::getResource();
  $res->setUri($doc_uri);
  $res->base = url('', array('absolute' => TRUE));
  $res->ns = rdfx_get_namespaces();

  // Non field API properties.
  if (!empty($node->rdf_mapping['rdftype'])) {
    $doc_props['rdf:type'] = $node->rdf_mapping['rdftype'];
  }
  if (!empty($node->rdf_mapping['title'])) {
    foreach ($node->rdf_mapping['title']['predicates'] as $p) {
      // @todo better way of typing literal or uris.
      $doc_props[$p] = $node->title;
    }
  }

  // Attach the node properties to the document URI.
  $index[$doc_uri] = $doc_props;
  // Expand all CURIEs and attach the index to the ARC2 resource.
  $res->index = $res->expandPNames($index);

  return $res;
}
