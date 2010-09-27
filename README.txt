RDF is a W3C standard for modeling and sharing distributed knowledge based on a
decentralized open-world assumption. This RDF package for Drupal 7 includes
several modules to enhance the RDF and RDFa functionnalities which are part of
Drupal 7 core.

- RDFx provides APIs for developers to manipulate RDF data, as well as output
  Drupal's data as RDF/XML, nTriples or Turtle.
- RDF UI allows site administrators to manage the RDF mappings of their site:
  alter default core mappings or specify mappings for the new content types and
  fields they create.
- Evoc enables the import of RDF vocabularies (such as FOAF, SIOC, etc.) which
  the site administrator can use to map Drupal data to RDF.


== Install the RDF module ==

  1. Copy all the module files into a subdirectory called
     sites/all/modules/rdf/ under your Drupal installation directory.

  2. Install the ARC2 library following one of these 2 options:
       - run "drush rdfx-arc2" (recommended, it will download the right package
         and extract it at the right place for you.)
       - manual install: download the library from
       http://arc.semsol.org/download and extract it in the rdf module directory
       such that you end up with the following file structure:
       sites/all/modules/rdf/vendor/arc/ARC2.php

  3. Go to Administer >> Site building >> Modules and enable the RDFx module and
     any other module you like. You will find them in the "RDF" section.


== Bug reports ==

Post bug reports and feature requests to the issue tracking system at:
<http://drupal.org/node/add/project_issue/rdf>


== Credits ==
The original RDF module was written for Drupal 6 by Arto Bendiken. It has been
refactored for Drupal 7 by Stéphane Corlosquet, based on his work with RDF CCK
and Evoc, which are now part of the main RDF package for Drupal 7.


== Current maintainers ==
  Stéphane "scor" Corlosquet - <http://openspring.net/>
  Lin Clark - <http://lin-clark.com/>
