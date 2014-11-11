<?php
/**
 * @file
 * contains OsImporterEntityValidateBase
 */

/**
 * Extending the entity validator class for plugins that just need to validate
 * the title and nothing more.
 */
class OsImporterEntityValidateBase extends EntityValidateBase {

  public function publicFieldsInfo() {
    $fields = parent::publicFieldsInfo();

    // Add all fields.
    foreach (field_info_instances($this->entityType, $this->bundle) as $field_name => $info) {
      if ($info['required']) {
        // Check if the 'isNotEmpty' validator is already assigned to the field
        // in the base class. Don't re-add it to avoid duplicated messages.
        if (isset($fields[$field_name]) && in_array('isNotEmpty', $fields[$field_name]['validators'])) {
          continue;
        }
        $fields[$field_name]['validators'][] = array($this, 'isNotEmpty');
      }
    }

    // Remove all validators for entityreference fields.
    foreach ($fields as $field_name => $info) {
      $field_info = field_info_field($field_name);
      $type = $field_info['type'];
      if ($type == 'entityreference') {
        unset($fields[$field_name]['validators']);
      }
    }

    // Remove the required field which not being imported via the importer or
    // taken care by the os_importer_entity_property_info_alter().
    $field_names = array(
      'field_news_date',
      'field_url',
      'field_links_link',
      'media_gallery_format',
      'media_gallery_image_info_where',
      'media_gallery_block_columns',
      'media_gallery_block_rows',
    );

    foreach ($field_names as $field) {
      unset($fields[$field]);
    }

    // Add path validator.
    $fields['path_alias']['validators'][] = array($this, 'validatorPath');

    return $fields;
  }

  /**
   * Verify the start is occurring before the end date.
   */
  public function validateOsDate($field_name, $value) {
    // Empty values are checked for in 'isNotEmpty'.
    if (empty($value)) {
      return;
    }

    $value = reset($value);
    // Validate the date format for the start and end date.
    $date = DateTime::createFromFormat('M j Y', $value);

    if ($date && $date->format('M j Y') == $value) {
      return;
    }

    $params = array(
      '@date' => $value,
      '@format' => date('M j Y'),
    );
    $this->setError($field_name, 'The value of the date field (@date) is not valid. The date should be in a format similar to @format.', $params);
  }

  /**
   * Validating the url field is not empty. We can't use the isNotEmpty since
   * the field is array with keys.
   */
  public function validatorUrlNotEmpty($field_name, $value) {
    $value = reset($value);
    if (empty($value)) {
      $this->setError($field_name, t('The field URL cannot be empty.'));
    }
  }

  /**
   * Validating the image minimum size.
   */
  public function validatorPhoto($field_name, $value, $min_width, $min_height) {
    list($width, $height) = getimagesize($value);

    if ($width < $min_width || $height < $min_height) {
      $params = array(
        '@width' => $width,
        '@height' => $height,
        '@min_width' => $min_width,
        '@min_height' => $min_height,
      );
      $this->setError($field_name, 'The size of the image need to be at least @min_widthX@min_height. The given image is @widthX@height', $params);
    }
  }

  /**
   * Validating the path alias.
   */
  public function validatorPath($field_name, $value) {
    if (empty($value)) {
      return;
    }

    $vsite = vsite_get_vsite();
    $purl = $vsite->group->purl;
    $alias_without_purl = $value[0];
    $alias_with_purl = $purl . '/' . $value[0];

    // Ensure that the alias does not exist yet.
    $query = db_select('url_alias')
      ->fields('url_alias')
      ->condition('alias', $alias_with_purl)
      ->condition('language', LANGUAGE_NONE)
      ->range(0, 1);

    if ($query->execute()->fetchField()) {
      // Alias already in use.
      $params = array(
        '@path' => $alias_without_purl,
      );
      $this->setError($field_name, t('The path "@path" is already in use. Please supply a different path.', $params));
    }

    // Check if the given alias is not taken by something more global.
    // @see _vsite_form_alias_validate().
    $items = array(
      'alias_with_purl' => menu_get_item($alias_with_purl),
      'alias_without_purl' => menu_get_item($alias_without_purl),
    );
    if ($items['alias_with_purl']['href'] == $alias_with_purl || $items['alias_without_purl']['href'] == $alias_without_purl) {
      $params = array(
        '@path' => $alias_without_purl,
      );
      $this->setError($field_name, t('The path "@path" is already in use. Please supply a different path.', $params));
    }
  }
}
