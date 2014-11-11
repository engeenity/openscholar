<?php

/**
 * @file
 * Contains OsImporterEventValidator
 */
class OsImporterEventValidator extends OsImporterEntityValidateBase {

  private $dates = array();

  public function publicFieldsInfo() {
    $fields = parent::publicFieldsInfo();

    $fields['field_date__start'] = array(
      'validators' => array(
        array($this, 'validateOsDate'),
      ),
    );

    $fields['field_date__end'] = array(
      'validators' => array(
        array($this, 'validateOsDate'),
      ),
    );

    $fields['registration'] = array(
      'validators' => array(
        array($this, 'validateSignUp'),
      ),
    );

    return $fields;
  }

  /**
   * Verify the value of the sign up is one of the: true, false, on, off.
   */
  public function validateSignUp($field_name, $value) {
    if (empty($value)) {
      return;
    }
    $value = reset($value);

    $values = array('true', 'false', 'on', 'off');
    if (!in_array(strtolower($value), $values)) {
      $params = array(
        '@value' => $value,
        '@values' => implode(", ", $values),
      );

      $this->setError($field_name, 'The field value (@value) should be one of the following values: @values', $params);
    }
  }

  /**
   * Verify the start is occurring before the end date.
   */
  public function validateOsDate($field_name, $value) {
    if (empty($value)) {
      return;
    }

    // Store the dates to compare start and end dates.
    $this->dates[$field_name] = reset($value);

    $value = reset($value);

    // Check if start date is greater then the end date.
    if (isset($this->dates['field_date__start']) && $field_name == 'field_date__end') {
      if (strtotime($this->dates['field_date__start']) > strtotime($this->dates['field_date__end'])) {
        $this->setError($field_name, 'The start date should not be greater than the end date.');
      }
    }

    // Check if only end date was supplied.
    if (empty($this->dates['field_date__start']) && $field_name == 'field_date__end') {
      $this->setError($field_name, 'The start date value should not be empty.');
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
    $this->setError($field_name, 'The value of the date field (@date) is not valid. The date should be in a format similar to @format.', $params);
  }
}
