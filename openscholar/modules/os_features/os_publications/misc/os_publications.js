Drupal.behaviors.osPublications = {
  attach: function (ctx) {
    // change the author category to the role when the form is submitted
    var $ = jQuery;
    $('.biblio-contributor-type .form-select', ctx).change(function (i) {
      var tar = $(this).parents('tr').find('.biblio-contributor-category input[type="hidden"]').not('.autocomplete');
      tar.val($(this).val());
    }).change();

    // Handle year fields.
    var codedYear = $("input[name='biblio_year_coded']");
    var yearField = $("#edit-biblio-year");

    // Add validation warning.
    var yearWarning = $("#biblio-year-group-validate");
    if (!yearField.hasClass('error')) {
      yearWarning.css('visibility', 'hidden');
    }
    yearWarning.css('color', 'red');

    // Allowed year input.
    var numbers = /^[0-9]+$/;

    // Publication year can be either given in a numerical value or by a coded
    // value ("in press", "submitted" and so on). If the user fills a numerical
    // value the radio buttons are unchecked and disabled. Clearing the numerical
    // value enables the radio buttons again.
    yearField.keyup(function() {
      if (this.value != '') {
        // Uncheck all radio buttons.
        codedYear.each(function () {
          $(this).prop('checked', false);
        });
        codedYear.prop("disabled", true);

        // Validate year input.
        userInput = this.value;
        if ((userInput.length != 4 && userInput.match(numbers)) || !userInput.match(numbers)) {
          yearWarning.css('visibility', 'visible');
          yearField.addClass("error");
        }
        else if (userInput.length == 4 && userInput.match(numbers)){
          yearWarning.css('visibility', 'hidden');
          yearField.removeClass("error");
        }
      }
      else {
        codedYear.prop("disabled", false);
        yearWarning.css('visibility', 'hidden');
        yearField.removeClass("error");
      }
    }).focus(function() {
      if ((yearField.value == '' || yearField.value == undefined) && !yearField.hasClass('error') ) {
        codedYear.prop("disabled", false);
      }
      else {
        codedYear.prop("disabled", true);
      }
    });
    codedYear.change(function() {
      if (this.value != '') {
        // Empty year field.
        yearField[0].value = '';
      }
    });
  }
};

(function ($) {
  // Override pathauto.js implementation to change summaries of the path field.
  Drupal.behaviors.pathFieldsetSummaries = {
    attach: function (context) {
      $('fieldset.path-form', context).drupalSetSummary(function (context) {
        var path = $('.form-item-path-alias input').val();
        var automatic = $('.form-item-path-pathauto input').attr('checked');

        if (automatic) {
          return Drupal.t('Automatic URL');
        }
        if (path) {
          return Drupal.t('URL: @alias', { '@alias': path });
        }
        else {
          return Drupal.t('No URL');
        }
      });
    }
  };

  /**
   * Manipulate the publications sort widgets pager.
   */
  Drupal.behaviors.PagerManipulation = {
    attach: function() {
      $(".next-page a, .prev-page a").click(function(event) {
        var parent = $(this).parents('.pager');
        var prev = parent.find('.prev-page');
        var next = parent.find('.next-page');
        var clicked = $(this);

        $.ajax({
          url: $(this).attr('href')
        })
        .success(function(result) {
            next.find('a').attr('href', result.next);
            prev.find('a').attr('href', result.prev);

            if (clicked.text() == next.find('a').text()) {
              if (result.next == 'none') {
                next.removeClass('visible').addClass('hidden');
                prev.removeClass('hidden').addClass('visible');
              }
              else {
                next.removeClass('hidden').addClass('visible');

                if (clicked != prev) {
                  prev.removeClass('hidden').addClass('visible');
                }
              }
            }

            if (clicked.text() == prev.find('a').text()) {
              if (result.prev == 'none') {
                prev.removeClass('visible').addClass('hidden');
                next.removeClass('hidden').addClass('visible');
              }
              else {
                prev.removeClass('hidden').addClass('visible');
                if (clicked != next) {
                  next.removeClass('hidden').addClass('visible');
                }
              }
            }

          $("#" + parent.attr('id')).html('<div class="item-list"><ul><li>' + result.results.join('</li><li>') + '</li></ul></div>');
        });
      });
    }
  };

})(jQuery);
