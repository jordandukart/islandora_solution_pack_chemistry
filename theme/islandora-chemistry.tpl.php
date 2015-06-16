<?php
/**
 * @file
 */
?>
<div class='islandora-chemistry'>
  <div class='image'><?php print $image; ?></div>
  <div class='viewer'><?php print $viewer; ?></div>
  <?php if (user_access(ISLANDORA_METADATA_EDIT)): ?>
    <div class='edit'><?php print l(t('edit'), "islandora/edit_form/{$islandora_object->id}/MODS"); ?></div>
  <?php endif; ?>
  <div class='metadata'><?php print $metadata; ?></div>
</div>
