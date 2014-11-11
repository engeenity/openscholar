<?php

/**
 * @file
 * Contains OsImporterLinkValidator
 */
class OsImporterLinkValidator extends OsImporterEntityValidateBase {

  public function publicFieldsInfo() {
    $fields = parent::publicFieldsInfo();

    $fields['field_links_link__url'] = array(
      'validators' => array(
        array($this, 'validatorUrlNotEmpty'),
      ),
    );

    return $fields;
  }
}
