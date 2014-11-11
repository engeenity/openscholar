<?php

/**
 * @file
 * Contains OsImporterFeedValidator
 */
class OsImporterFeedValidator extends OsImporterEntityValidateBase {

  public function publicFieldsInfo() {
    $fields = parent::publicFieldsInfo();

    $fields['field_url__url'] = array(
      'validators' => array(
        array($this, 'validatorUrlNotEmpty'),
      ),
    );

    return $fields;
  }
}
