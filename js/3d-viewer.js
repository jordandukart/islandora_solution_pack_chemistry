(function ($) {
  Drupal.behaviors.islandora_chemistry_3d_view = {
    attach: function (context, settings) {
      var chem_settings = settings.islandora_chemistry_3d_view;
      $(chem_settings.element_selector, context).once('islandora_chemistry_3d_view', function (index, element) {
        var canvas = new ChemDoodle.TransformCanvas3D(element.id, chem_settings.width, chem_settings.height);
        canvas.specs.set3DRepresentation("Ball and Stick");
        canvas.specs.backgroundColor = "black";
        canvas.specs.atoms_displayLabels_3D = true;
        var molecule = ChemDoodle.readMOL(chem_settings.mol_file, 1);
        canvas.loadMolecule(molecule);
      });
    }
  };
})(jQuery);
