About

Panelizer allows you to treat any node type as a panel node. You can provide default panels, per node type, and control both the available content and available layouts, also per node type.

Installing

Install this through the normal Drupal method of putting the module in sites/all/modules and going to admin/build/modules to activate it.

It requires Panels and Page Manager.

Initial configuration

Visit administer >> settings >> panelizer to enable the module for the node types you need. 

Ensure that the node template system page is enabled.

API

@todo
  drupal_alter('panelizer_default_types', $types, 'node');

Future functionality
  - Allow panels for different build modes -- this is tricky
  - Fully implement the ability to choose from different defaults and restrict custom panel per node
  - Implement user panelizer
  - Implement panel subtabs. i.e, allow node/27/arbitrarylink to be a subtab of a node, using panelizer.
