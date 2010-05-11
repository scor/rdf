<?php

/**
 * Loads an RDF/XML file from an HTTP URI, parses it, and builds an RDF schema
 * representation (associative array) from it.
 * Handles content negotiation and various errors.
 *
 * @param string $uri
 * @return array
 * @throws Exception On network or parse error
 */
function evoc_get_from_web($uri, $prefix) {
  $schema_url = $uri;
  $i = strpos($schema_url, '#');
  if ($i !== false) {
    $schema_url = substr($schema_url, 0, $i);
  }
  list($triples, $namespaces) = _evoc_parse_rdfxml($uri);
  return _evoc_extract_schema($triples, $namespaces, $prefix, $uri);
}

/**
 * Loads a local RDF/XML file, parses it, and builds
 * an RDF schema representation (associative array) from it.
 * Handles various errors.
 *
 * @param string $uri
 * @return array
 * @throws Exception On file or parse error
 */
function evoc_get_from_file($filename, $uri, $prefix) {
  $content = _evoc_fetch_from_file($filename);
  list($triples, $namespaces) = _evoc_parse_rdfxml($uri, $content);
  return _evoc_extract_schema($triples, $namespaces, $prefix, $uri);
}

/**
 * Loads the content of a local file.
 *
 * @param string $filename
 * @return string
 * @throws Exception
 */
function _evoc_fetch_from_file($filename) {
  if (!is_file($filename)) {
    throw new Exception("File not found: '$filename'");
  }
  return file_get_contents($filename);
}

function &_evoc_parse_rdfxml($base_uri) {
  include_once(drupal_get_path('module', 'rdfx') . '/vendor/arc/ARC2.php');
  $parser = ARC2::getRDFXMLParser();
  $parser->parse($base_uri);
  $namespaces = array();
  foreach ($parser->nsp as $uri => $prefix) {
    $namespaces[$prefix] = $uri;
  }
  return array($parser->getTriples(), $namespaces);
}

function _evoc_load__error_handler($errno, $errstr) {
    throw new Exception($errstr);
}

/**
 * Queries a set of triples for classes and properties, and builds
 * an associative array describing the vocabulary and any 
 * classes and properties found.
 *
 * @param array $model An ARC2-style array of triples an RDFS vocabulary or OWL ontology
 * @param array $namespaces Associative array of namespaces parsed from the RDF file
 * @param string $ns_prefix Namespace prefix for the vocabulary
 * @param string $ns_uri Only terms in this namespace will be considered
 * @return array Array describing the vocabulary, its classes and properties.
 */
function _evoc_extract_schema(&$model, $namespaces, $ns_prefix, $ns_uri) {
  $vocabulary = array(
      'prefix' => $ns_prefix,
      'uri' => $ns_uri,
      'title' => _evoc_query_for_title($model, $ns_uri),
      'description' => _evoc_query_for_description($model, $ns_uri),
      'classes' => array(),
      'properties' => array(),
      'namespaces' => $namespaces,
  );
// @todo Remove cruft, make this generalized.
  // Add description of each property to $vocabulary['classes']
  $terms = _evoc_fetch_terms($model);
  dpm($terms);
 foreach ($property_uris as $uri) {
     /*if (substr($uri, 0, strlen($ns_uri)) != $ns_uri) {
      $ignored_uris[] = $uri;
      continue;
    }*/
    // @todo Make this work.
    $vocabulary['properties'][] = _evoc_query_for_property_description($model, $uri);
  }

  return $vocabulary;
}

function _evoc_query_for_title(&$model, $ns_uri) {
  $queries = array(
    array($ns_uri, 'http://www.w3.org/2000/01/rdf-schema#label', '?'),
    array($ns_uri, 'http://purl.org/dc/elements/1.1/title', '?'),
    array($ns_uri, 'http://purl.org/dc/terms/title', '?'),
  );
  if (substr($ns_uri, -1) == '#') {
    $uri = substr($ns_uri, 0, -1);
    $queries = array_merge($queries, array(
      array($uri, 'http://www.w3.org/2000/01/rdf-schema#label', '?'),
      array($uri, 'http://purl.org/dc/elements/1.1/title', '?'),
      array($uri, 'http://purl.org/dc/terms/title', '?'),
    ));
  }
  return _evoc_query_find_literal($model, $queries);
}

function _evoc_query_for_description(&$model, $ns_uri) {
  $queries = array(
    array($ns_uri, 'http://www.w3.org/2000/01/rdf-schema#comment', '?'),
    array($ns_uri, 'http://purl.org/dc/elements/1.1/description', '?'),
    array($ns_uri, 'http://purl.org/dc/terms/description', '?'),
  );
  if (substr($ns_uri, -1) == '#') {
    $uri = substr($ns_uri, 0, -1);
    $queries = array_merge($queries, array(
      array($uri, 'http://www.w3.org/2000/01/rdf-schema#comment', '?'),
      array($uri, 'http://purl.org/dc/elements/1.1/description', '?'),
      array($uri, 'http://purl.org/dc/terms/description', '?'),
    ));
  }
  return _evoc_query_find_literal($model, $queries);
}

function _evoc_fetch_terms(&$model) {
  $terms = array();
  
  $term_type_groups = rdfx_term_types();
  foreach($term_type_groups as $term_type_groups => $term_types) {
    foreach ($term_types as $term_type => $metadata) {
      $query = array(
        array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', $metadata['uri']),
        array('?', $metadata['inference']['subject'], null),
        array('?', $metadata['inference']['object'], null),
      );
      $terms[$term_type] = _evoc_query_find_uris($model, $query);
    }
  }
  return $terms;
}

function _evoc_query_for_property_uris(&$model) {
  // @todo Move all of the additional classes and property inferences to the
  // rdfx_term_types array and delete this function.
  
  /*return _evoc_query_find_uris($model, array(
    //array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#Property'),
    //array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#DatatypeProperty'),
    //array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#ObjectProperty'),
    //array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#FunctionalProperty'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#InverseFunctionalProperty'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#SymmetricProperty'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#AnnotationProperty'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#ReflexiveProperty'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#IrreflexiveProperty'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#AsymmetricProperty'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#TransitiveProperty'),
    array('?', 'http://www.w3.org/2000/01/rdf-schema#domain', null),
    array('?', 'http://www.w3.org/2000/01/rdf-schema#range', null),
    array('?', 'http://www.w3.org/2000/01/rdf-schema#subPropertyOf', null),
    array(null, 'http://www.w3.org/2000/01/rdf-schema#subPropertyOf', '?'),
    array('?', 'http://www.w3.org/2002/07/owl#equivalentProperty', null),
    array(null, 'http://www.w3.org/2002/07/owl#equivalentProperty', '?'),
    array('?', 'http://www.w3.org/2002/07/owl#inverseOf', null),
    array(null, 'http://www.w3.org/2002/07/owl#inverseOf', '?'),
  ));*/
}

function _evoc_query_for_term_description(&$model, $uri) {
  $label = _evoc_query_find_literal($model, array(
    array($uri, 'http://www.w3.org/2000/01/rdf-schema#label', '?')
  ));

  $comment = _evoc_query_find_literal($model, array(
    array($uri, 'http://www.w3.org/2004/02/skos/core#definition', '?'),
    array($uri, 'http://www.w3.org/2000/01/rdf-schema#comment', '?'),
  ));

  //$qname = _evoc_uri_to_qname($uri);
  ////print_r('<br>Term Descrip' . $uri . '<br>');
  return array(
      'uri' => $uri,
      'label' => $label,
      'comment' => $comment,
  );
}

function _evoc_query_for_class_description(&$model, $uri) {
  $class = _evoc_query_for_term_description($model, $uri);
  $class['superclasses'] = _evoc_query_find_qnames($model, array(
    array($uri, 'http://www.w3.org/2000/01/rdf-schema#subClassOf', '?')
  ));
  $class['disjoints'] = _evoc_query_find_qnames($model, array(
    array($uri, 'http://www.w3.org/2002/07/owl#disjointWith', '?'),
    array('?', 'http://www.w3.org/2002/07/owl#disjointWith', $uri),
  ));
  return $class;
}

function _evoc_query_for_property_description(&$model, $uri) {
  ////print_r('<br>' . $uri . '<br>');
  $property = _evoc_query_for_term_description($model, $uri);
  ////print_r('<br>' . $uri . '<br>');
  $property['domains'] = _evoc_query_find_qnames($model, array(
    array($uri, 'http://www.w3.org/2000/01/rdf-schema#domain', '?')
  ));
  $property['ranges'] = _evoc_query_find_qnames($model, array(
    array($uri, 'http://www.w3.org/2000/01/rdf-schema#range', '?')
  ));
  $property['superproperties'] = _evoc_query_find_qnames($model, array(
    array($uri, 'http://www.w3.org/2000/01/rdf-schema#subPropertyOf', '?')
  ));
  $property['is_functional'] = _evoc_query_ask($model, array(
    array($uri, 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#FunctionalProperty')
  ));
  $property['is_inverse_functional'] = _evoc_query_ask($model, array(
    array($uri, 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#InverseFunctionalProperty')
  ));
  $property['inverses'] = _evoc_query_find_qnames($model, array(
    array($uri, 'http://www.w3.org/2002/07/owl#inverseOf', '?'),
    array('?', 'http://www.w3.org/2002/07/owl#inverseOf', $uri),
  ));
  //print_r($property);

  // @todo TODO We don't handle multiple inverses correctly everywhere yet, so store it both as an array and singleton
  $property['inverse'] = count($property['inverses']) ? $property['inverses'][0] : null;
  return $property;
}

function _evoc_query_ask(&$model, $queries) {
  foreach ($queries as $query) {
    list($s, $p, $o) = $query;
    if (_evoc_query_find_first($model, $s, $p, $o)) return true;
  }
  return false;
}

function _evoc_query_find_literal(&$model, $queries) {
  foreach ($queries as $query) {
    list($s, $p, $o) = $query;
    $triple = _evoc_query_find_first($model, $s, $p, $o);
    if (!$triple) continue;
    if ($triple['o_type'] == 'literal') return $triple['o'];
  }
  return null;
}

function _evoc_query_find_uris(&$model, $queries) {
  $uris = array();
  foreach ($queries as $query) {
    list($s, $p, $o) = $query;
    $result = _evoc_query_find_all($model, $s, $p, $o);
    foreach ($result as $triple) {
      if ($s == '?' && $triple['s_type'] == 'uri') {
        $uris[] = $triple['s'];
      }
      if ($p == '?') {
        $uris[] = $triple['p'];
      }
      if ($o == '?' && $triple['o_type'] == 'uri') {
        $uris[] = $triple['o'];
      }
    }
  }
  return array_unique($uris);
}

function _evoc_query_find_qnames(&$model, $queries) {
  $uris = _evoc_query_find_uris($model, $queries);
  $qnames = array();
  foreach ($uris as $uri) {
    $qnames[] = $uri;
  }
  return $qnames;
}

function _evoc_query_find_first(&$model, $s, $p, $o) {
  foreach ($model as $triple) {
    if (!is_null($s) && $s != '?' && ($triple['s'] != $s || $triple['s_type'] != 'uri')) continue;
    if (!is_null($p) && $p != '?' && ($triple['p'] != $p)) continue;
    if (!is_null($o) && $o != '?' && ($triple['o'] != $o || $triple['o_type'] != 'uri')) continue;
    return $triple;
  }
  return null;
}

function _evoc_query_find_all(&$model, $s, $p, $o) {
  $result = array();
  foreach ($model as $triple) {
    if (!is_null($s) && $s != '?' && ($triple['s'] != $s || $triple['s_type'] != 'uri')) continue;
    if (!is_null($p) && $p != '?' && ($triple['p'] != $p)) continue;
    if (!is_null($o) && $o != '?' && ($triple['o'] != $o || $triple['o_type'] != 'uri')) continue;
    $result[] = $triple;
  }
  return $result;
}

/**
 * Abbreviates a URI into a QName, taking into account any namespaces
 * registered by modules, and any namespaces defined in the RDF model
 * currently being imported. Will generate new namespace mappings if
 * necessary.
 *
 * @param string $uri
 * @return string
 */
function _evoc_uri_to_qname($uri) {
  global $evoc_import_namespaces;
  $longest_match = '';
  foreach ($evoc_import_namespaces as $ns_uri => $ns_prefix) {
    if (substr($uri, 0, strlen($ns_uri)) == $ns_uri && strlen($ns_uri) > strlen($longest_match)) {
      $longest_match = $ns_uri;
    }
  }
  if ($longest_match) {
    $qname = $evoc_import_namespaces[$longest_match]['prefix'] . ':' . substr($uri, strlen($longest_match));
    $evoc_import_namespaces[$longest_match]['used'] = true;
    return $qname;
  }
  // The prefix is not known. Split it into NS and local.
  if (!preg_match('!^(.*[:/#])[^:/#]*$!', $uri, $match)) {
    throw new Exception('Could not generate namespace mapping for URI ' . $uri);
  }
  $ns = $match[1];
  // Use the RDF API to generate a namespace prefix for us (g.1, g.2, etc)
  // Note: We do the splitting ourselves above because the RDF API function
  // doesn't have the proper regex, it doesn't allow certain characters such
  // as dots in the local name, resulting in funky namespace URIs such as
  // <http://...../foo.owl#Foo.>.
  $qname = rdf_uri_to_qname($ns . 'local');
  if (!$qname) {
    throw new Exception('Could not generate namespace mapping for URI ' . $uri);
  }
  $evoc_import_namespaces[$ns]['prefix'] = substr($qname, 0, strpos($qname, ':'));
  $evoc_import_namespaces[$ns]['new'] = true;
  $evoc_import_namespaces[$ns]['used'] = true;
  return $qname;
}