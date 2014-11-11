<?php

/**
 * @file
 * Contains OsImporterPresentationValidator
 */
/**
 * required title
 * validate date
 */
class OsImporterPresentationValidator extends OsImporterEntityValidateBase {

  public function publicFieldsInfo() {
    $fields = parent::publicFieldsInfo();

    $fields['field_presentation_date__start'] = array(
      'validators' => array(
        array($this, 'validateOsDate'),
      ),
    );

    return $fields;
  }

  /**
   * Overrides OsImporterEntityValidateBase::validateOsDate() to allow empty
   * date value.
   */
  public function validateOsDate($field_name, $value, EntityMetadataWrapper $wrapper, EntityMetadataWrapper $property_wrapper) {
    $value = reset($value);

    // We allow an empty date value for this content type.
    if (empty($value)) {
      return;
    }

    // Validate the date format for the start and end date.
    $date = DateTime::createFromFormat('M j Y', $value);

    if ($date && $date->format('M j Y') == $value) {
      return;
    }

    $params = array(
      '@date' => $value,
      '@format' => date('M j Y'),
    );
    $this->setError($field_name, 'The value in the date field (@date) is not valid. The date should be in a format similar to @format.', $params);
  }
}
