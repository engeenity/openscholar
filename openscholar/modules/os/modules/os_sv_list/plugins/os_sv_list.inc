<?php

class os_sv_list extends os_boxes_default {
  
  private $plugins = array();
  protected $entity_type;
  protected $bundle;
  
  public function __construct() {
    parent::__construct();
    $this->entity_type = $this->bundle = NULL;
  }
  
  /**
   * Implementation of boxes_content::options_defaults().
   */
  public function options_defaults() {
    $options = parent::options_defaults();

    $options += array(
      'content_type' => 'all',
      'display' => 'teaser',
      'layout' => 'list',
      'grid_size' => 3,
      'more' => FALSE,
      'more_text' => t('More'),
      'more_link' => '',
  
      'pager' => FALSE,
      'number_of_items' => 6,
      'sort_by' => 'sort_newest',
    );

    //get options from plugins
    if (empty($this->plugins)) {
      $this->_set_plugins($this->entity_type);
    }
    
    foreach ($this->plugins as $plugin) {
      if (method_exists($plugin['instance'], 'options_defaults')) {
        $options += $plugin['instance']->options_defaults();
      }
    }
    
  
    return $options;
  }
  
  /**
   * Implementation of boxes_content::options_form().
   */
  public function options_form(&$form_state) {
    $form = parent::options_form($form_state);
    $this->options = ($this->options) ? array_merge($this->options_defaults(), $this->options) : $this->options_defaults();
    
    $form['#submit'] = array('os_sv_list_form_submit');
    $form['#attached'] = array(
      'js' => array(libraries_get_path('select2') . '/select2.js'),
      'css' => array(
        libraries_get_path('select2') . '/select2.css',
        drupal_get_path('module', 'os_sv_list') . '/os_sv_list.css',
      ),
    );

     $form['filters'] = $form['details'] = $form['content'] = array(
      '#type' => 'fieldset',
      '#collapsible' => FALSE,
    );

    
    //content fieldset
    $form['content']['content_type'] = array(
      '#type' => 'select',
      '#title' => t('Content Type'),
      '#id' => 'os_sv_list_content_type',
      '#default_value' => $this->options['content_type'],
      '#attached' => array(
        'js' => array(
          drupal_get_path('module', 'os_sv_list').'/os_sv_list.js',
          array('type' => 'setting', 'data' => array('entity_view_modes' => $this->entity_view_modes())),
        ),
      ),
    );
    
    $form['content']['display'] = array(
      '#type' => 'select',
      '#title' => 'Display style',
      '#default_value' => $this->options['display'],
      '#options' => $this->_display_options(),
      // Subsite content can only show titles, so hide this element when subsites are active
      '#states' => array(
        'visible' => array(// action to take.
          ':input[name="query_scope[subsites]"]' => array('checked' => FALSE),
        ),
      ),
    );
    

    $form['content']['sort_by'] = array(
      '#type' => 'select',
      '#title' => 'Sorting',
      '#default_value' => $this->options['sort_by'],
      '#options' => $this->_sort_options(),
      '#attached' => array(
        'js' => array(
          array('type' => 'setting', 'data' => array('sv_list_sort' => $this->_sort_options_type_bundles())),
        )
      )
    );
  


    //details fieldset
    $form['details']['more'] = array(
      '#title' => 'Show "more" link',
      '#type' => 'checkbox',
      '#default_value' => $this->options['more'],
    );
 
    if (!empty($form_state['values']['more_link'])) {
    // Unset the values from the text field, as otherwise FAPI will add
    // it to the #value of the form-element.
    // @see http://drupal.org/node/1024962#comment-4021826
        unset($form_state['input']['more_link']);
    }
    
    $form['details']['more_link'] = array(
      '#title' => t('URL for the more link'),
      '#type' => 'textfield',
      '#default_value' => $this->options['more_link'],
      '#states' => array(
        'visible' => array('input[name=more]' => array('checked' => TRUE)),
        'invisible'  => array('input[name=more]' => array('checked' => FALSE)),
      ),
      '#prefix' => '<div id="more_link_div">',
      '#suffix' => '</div>',
      '#attached' => array(
        'js' => array(
          array('data' => array('more_link_defaults' => $this->_more_link_defaults(),),'type' => 'setting',)
        )
      )
    );
    
    $form['details']['more_text'] = array(
      '#title' => 'Label of the more link (default: "more")',
      '#type' => 'textfield',
      '#default_value' => $this->options['more_text'],
        '#states' => array(
          'visible' => array('input[name=more]' => array('checked' => TRUE)),
        ),
    );
    
    $form['details']['pager'] = array(
      '#title' => 'Show pager',
      '#type' => 'checkbox',
      '#default_value' => $this->options['pager'],
    );
  
    $form['details']['number_of_items'] = array(
      '#title' => 'Number of items to display',
      '#type' => 'select',
      '#options' => array_slice(range(0, 20), 1, NULL, TRUE),
      '#default_value' => $this->options['number_of_items'],
    );
    
    //get options from plugins
    $this->_plugins_invoke('options_form', $form);

    //hide empty fieldsets.  plugins _could_ empty some of the defaults
    foreach (array('details', 'content', 'filters') as $fieldset) {
      if (isset($form[$fieldset]) && count($form[$fieldset]) <= 2) {
        unset($form[$fieldset]);
      }
    }

    return $form;
  }
    
  public function render() {
    if (module_exists('vsite') && !vsite_get_vsite()) {
      return;
    }
    
    $bundle = ($this->options['content_type'] != 'all') ? $this->options['content_type'] : NULL;
    $this->_set_plugins($this->entity_type, $bundle);
    
    $block = parent::render();
    $this->options += $this->options_defaults();
    $page_num = (isset($_GET['page'])) ? $_GET['page'] : 0;
    $page = 0;
    $ids = $this->get_ids($page_num, $page);
   
    //add the pager
    if ($this->options['pager'] && $ids) {
      $parameters = array(
        'sv_list_box_delta' => $this->delta,
        'pager_id' => $this->pager_element,
        'destination' => $_GET['q'],
      );
      $use_pager = 'views_mini_pager';
      $pager = theme($use_pager , array(
        'parameters' => $parameters,
        'element' => $this->pager_element,
        'quantity' => $this->options['number_of_items'],
      ));
      
      //if the pager is active, the more link needs its vars to know whether or not to display.
      global $pager_page_array, $pager_total;
      $pager_current = $pager_page_array[$parameters['pager_id']] + 1;
      $pager_max = $pager_total[$parameters['pager_id']];
    } else {
      $pager = '';
    }
        
    //truncate items and prepare the more link
    $more = '';
    if (count($ids) > $this->options['number_of_items'] || ($this->options['pager'] && $ids && $pager_current < $pager_max)) {
      $ids = array_slice($ids, 0, $this->options['number_of_items']);
      $more = ($this->options['more']) ? l($this->options['more_text'], $this->options['more_link'], array('attributes' => array('class' => array('more-link')))) : '';
    }
    
    //give plguins a chance to alter the entities after loading but before rendering.  see events.
    $entities = entity_load($this->entity_type, $ids);
    $this->_plugins_invoke('entities_alter', $entities);
    if (empty($entities)) {
      return $block;
    }
    
    //let a plugin do the rendering.  if they don't, use the default.
    $render = $this->_plugins_invoke('render', $entities);
    $render = ($render) ? $render : entity_view($this->entity_type, $entities, $this->options['display']);
    
    //apply a layout.  just doing grid for now.
    $layout = 'sv-list-' . $this->options['layout'];
    if ($this->options['layout'] == 'grid') {
      $layout .= ' sv-list-grid-' . $this->options['grid_size'];
      $render = (isset($render[$this->entity_type])) ? $render[$this->entity_type] : $render; 
      $render['#attached']['css'][] = drupal_get_path('module', 'os_sv_list').'/os_sv_list_layout.css';
      
      //if the nth-child css doesn't work out, we could manually insert a div...
//       $div = array('#markup' => '<div class="sv-list-new-row">DIV!</div>', '#type' => 'markup');
//       $c = count($entities);
//       for ($i = $this->options['grid_size'] ; $i < count($entities); $i += 1 + $this->options['grid_size']) {
//         array_splice($render, $i, 0, array($div));
//       }
//       $render = (isset($render[$this->entity_type])) ? $render[$this->entity_type] : $render;
//       foreach ($render as &$item) {
//         if (is_array($item)) {
//           $item['#attributes'] = array('class' => array('sv-grid'));
//         }
//       }    
    }
    
    //now render to html.
    $content = drupal_render($render);      
    $block['content'] = '<div id="box-' . $this->delta . '-page" data-page="'.$page_num.'" data-delta="'.$this->delta.'" class="'. $layout .'">' . $content . '</div>' . $pager . $more;
    return $block;
  }
  
  public function get_ids($page_num, $page) {    
    //query for all content of our type
    $query = $this->query();
    $this->_plugins_invoke('query_alter', $query);
    
    if ($this->options['content_type'] != 'all') {
      $query->propertyCondition('type', $this->options['content_type']);
      //any special hook here?
    }
    
    
    
    //pager
    if ($this->options['pager'] && is_int($page)) {
      $query->pager($this->options['number_of_items']);
      $this->pager_element = $query->pager['element'];
    } elseif (is_int($page)) {
      $query->range(0, 1 + $this->options['number_of_items']); //add one so more link knows there's more.
    }
    
    //sort - if $this doesn't have it, one of the plugins will.
    $sort = $this->options['sort_by'];
    if (method_exists($this, $sort)) {
      $this->$sort($query);
    } else {
      $this->_plugins_invoke($sort, $query);
    }
    $this->_plugins_invoke('sort_alter', $query);
    
    //person, !image
    //event exception.  dletas.

    
    //execute
    $result = $query->execute();
    return (isset($result[$this->entity_type])) ? array_keys($result[$this->entity_type]) : array();
  }
  
  public function query() {
    $efq = new EntityFieldQuery();
    $efq->entityCondition('entity_type', $this->entity_type);
    
    if (module_exists('vsite') && $space = spaces_get_space()) {
      $efq->fieldCondition(OG_AUDIENCE_FIELD, 'target_id', $space->id);
    }
    
    return $efq;
  }
  
  
  /**
   * @function _set_plugins
   * 
   * Instantiates all the plugins this box will use.  Call this on demand instead of on __construct,
   * because the plugins loaded will depend on context.  A node box has a fixed entity_type.  During
   * render, its bundle is fixed, but in the options form, its bundle can change.
   * 
   * Re-run with new criteria to reset.  This way if a box's option form loads all options for its
   * entity_type, the render method can reset to only those applicable to the bundle.   
   */
  private function _set_plugins($entity_type = NULL, $bundle = NULL) {
    $this->plugins = array();
    
    foreach (os_sv_list_plugins($entity_type, $bundle) as $plugin) {
      $class = ctools_plugin_get_class($plugin, 'handler');
      $plugin['instance'] = new $class($entity_type, $bundle);
      $this->plugins[$plugin['name']] = $plugin;
    }
    
    return $this->plugins;
  }
  
  /**
   * @function _plugins_invoke
   *
   * Runs a method on all loaded plugins.  Plugin return values will return in
   * keyed array.
   */
  private function _plugins_invoke($method, &$args = NULL) {
    $options = ($this->options) ? $this->options + $this->options_defaults() : $this->options_defaults();
    foreach ($this->plugins as $name => $plugin) {
      if (method_exists($plugin['instance'], $method)) {
        $ret[$plugin['name']] = $plugin['instance']->$method($options, $args); //to pass by ref we can't use func_get_args
      }
    }
    
    return (isset($ret)) ? array_filter($ret) : array();
  }
  
  /**
   * @function entity_view_modes
   * 
   * Wrapper for os_entity_view_modes.  Also derives common view modes
   * available to all content types.
   */
  private function entity_view_modes() {
    $modes = os_entity_view_modes($this->entity_type);
    $all = array_reduce($modes, 'array_intersect', current($modes)); //starting with an empty array means we'll get an empty back.
    $modes['all'] = array_values($all);  
    return $modes;
  }
    
  protected function _display_options() {
    $defaults = array(  //Display modes will be shown in this order.  If no order is provided, it'll be determined by how the hooks load them.
      'node' => array('title', 'teaser', 'full'),
      'file' => array('os_files_link', 'os_files_link_icon', 'os_files_title', 'teaser', 'full'),
    );
    $defaults = (isset($defaults[$this->entity_type])) ? $defaults[$this->entity_type] : array();
    
    //list all the available view modes for this entity
    $view_modes = os_entity_view_modes($this->entity_type);
    $view_modes = array_unique(array_reduce($view_modes, 'array_merge', $defaults)); 
    
    //get labels for these view modes
    $info = entity_get_info($this->entity_type);
    $options = array_reduce($view_modes, function($ret, $mode) use ($info) {$ret[$mode] = $info['view modes'][$mode]['label']; return $ret;});
    
    //rename some options to friendlier titles
    array_merge(array('full' => 'Entire Post'), $options);
    return $options;    
  }
  
  /**
   * @function _more_link_defaults()
   * 
   * Gets the defaults that fill in for the more link when the content type changes
   */
  protected function _more_link_defaults() {
    //more link.  still needs some refactoring love.
    //populate list of content types => paths
    ctools_include('os', 'os');
    $content_types = array('all' => t('All')) + os_get_bundles(array(1));
    
    $content_map = os_get_app_path();
    $content_map = array_diff_key($content_map, array_flip(array('bio', 'cv', 'class_material')));
    if (isset($content_types['image'])) {
      $content_map['image'] = $content_map['media_gallery'];
    }
    
    // Use blank path for unknown paths.
    $content_map += array_fill_keys(array_keys(array_diff_key($content_types, $content_map)),' ');
    return $content_map;     
  }
  
  /**
   * @function _sort_options()
   * 
   * Returns hash of sort options and their labels
   */
  protected function _sort_options() {
    
//     $type_sorts = array();
//     $sorts = array();
//     foreach ($this->_plugins_invoke('register_sorts') as $plugin => $sort) {
//       $info = $this->plugins[$plugin];
//       $sorts += $sort;
//       $types = ($info['entity_type']) ? $info['entity_type'] : array('all');
//       $bundles = ($info['bundle']) ? $info['bundle'] : array('all');
      
//       foreach ($types as $type) {          
//         foreach ($bundles as $bundle) {
//           $type_sorts[$type][$bundle] = (!isset($type_sorts[$type][$bundle])) ? $sort : array_merge($type_sorts[$type][$bundle], $sort); 
//         } 
//       }
      
//     }
    $sorts = array_reduce($this->_plugins_invoke('register_sorts'), 'array_merge', array());
    
    return $sorts;
//     return array(
//       // Content type sensitive sorts.  // in the future get these from sv render plugins
//       'sort_event' => t('Event Date'),
//       'sort_presentation' => t('Recently Presented'),
//       'sort_biblio' => t('Year of Publication'),
//       'sort_news' => t('News Date'),
    
//       // Standard sorts,
//       'sort_newest' => t('Newest Post'),
//       'sort_oldest' => t('Oldest Post'),
//       'sort_alpha' => t('Alphabetical'),
//       'sort_random' => t('Random'),
//     );
  }
  
  /**
   * @function _sort_options_type_bundles
   * 
   * Returns list of sort options, and the entities/bundles that can use them.
   * Items not appearing here are universal.
   */
  function _sort_options_type_bundles() {
    $return = array();
    foreach ($this->_plugins_invoke('register_sorts') as $plugin => $sorts) {
      $plugin = $this->plugins[$plugin];
      $return += array_fill_keys(array_keys($sorts), array_filter(array('entity_type' => $plugin['entity_type'], 'bundle' => $plugin['bundle'])));
    }
    
    return $return;
  }

  function sort_random(&$efq) {
    $efq->addTag('random');
  }
  
}