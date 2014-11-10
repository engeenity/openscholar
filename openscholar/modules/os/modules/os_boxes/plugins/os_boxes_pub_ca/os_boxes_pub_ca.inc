<?php

/**
 * @file
 * The class for "Publication By Year" type widgets.
 */

class os_boxes_pub_ca extends os_boxes_default {

  public $embed_as = array(
    'iframe',
  );

  /**
   * Implementation of boxes_content::options_defaults().
   */
  public function options_defaults() {
    $options = parent::options_defaults();

    $options['display_num'] = 1;

    return $options;
  }

  /**
   * Implementation of boxes_content::options_form().
   */
  public function options_form(&$form_state) {
    $form = parent::options_form($form_state);

    $form['display_num'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display number of items in category.'),
      '#default_value' => $this->options['display_num'],
    );

    return $form;
  }

  /**
   * Implementation of boxes_content::render().
   */
  public function render() {
    $block = parent::render();
    $this->set_cache_id('biblio');
    if (($block['content'] = $this->get_cache()) === FALSE) {
      $block['content'] = os_publications_sort_publication_block('author', 'co_authors', $this->delta);
      $this->set_cache($block['content']);
    }

    return $block;
  }

  /**
   * Return the cache parameters for this box
   */
  public function cache_info(){
    // Caching disabled so ajax pager gets init'ed.
  	return array();
  }

}
