// $Id: 

(function ($) {

  Drupal.behaviors.predicateSelection = {
    attach: function (context) {

      $('.predicate-store').each(function(index) {
        wrapper = $(this).parents('div.form-item.form-type-textarea');
        wrapper.data('changed',false);
        $(this).parents('.resizable-textarea').hide();
        $(this).hide();
        $(this).after(Drupal.theme('rdfPredicatesWidget', wrapper, index));
        initPredicates(wrapper);
      });
      
      $('.predicate-add').click(function(){
        wrapper = $(this).parents('div.form-item.form-type-textarea');
        val = $(this).prev().val();
        if ($(wrapper).find('.predicate-val').filter(function(){
          return $(this).text() == val;
        }).length == 0) {
          pattern = eval(Drupal.settings.rdfui.predicateRegex);
          if (pattern.test(val)) {
            $(wrapper).find('.predicate-holder').append(Drupal.theme('rdfPredicate',val, false));
            bindRemoveClicks(wrapper);
            updateStore(wrapper);
            $(this).prev().val('');        
          }
          else {
            alert(Drupal.t(val + ' is an invalid format.'));
          }
        }  
        
        return false;
      });
      
      function initPredicates(wrapper) {
        textarea = wrapper.find('.predicate-store');
        wrapper.find('.predicate-holder').children().remove();
        jQuery.each($(textarea).html().split('\n'), function(i,val) {
          if (jQuery.trim(val) != '') {
            $(wrapper).find('.predicate-holder').append(Drupal.theme('rdfPredicate',val, true));
          }
        });
        bindRemoveClicks(wrapper);
      }
      
      function updateStore(wrapper) {
        textarea = wrapper.find('.predicate-store');
        textarea.html('');
        wrapper.find('.predicate-val').each(function(){
          textarea.append($(this).html() + '\n');
        });
        
        if(wrapper.data('changed') == false) {
          $(wrapper).find('.predicates-widget').prepend(Drupal.theme('rdfPredicatesChangedWarning')).hide().fadeIn();
          wrapper.data('changed',true);
        }
      }
      
      function bindRemoveClicks(wrapper) {
        $(wrapper).find('.predicate-remove:not(.predicate-process)').click(function(){
          $(this).parent().remove();
          updateStore(wrapper);          
        }).addClass('.predicate-process');

      }
            
    }
  };
  
  Drupal.theme.prototype.rdfPredicate = function(val, saved) {
    saved_class = '';
    saved_text = '';
    if (!saved) {
      saved_class = ' predicate-unsaved';
      saved_text = '<span class="warning">*</span>';
    }
    return '<span class="predicate'+ saved_class +'"><span class="predicate-val">' + val + '</span>'+
    saved_text +' <span class="predicate-remove">x</span></span>';
  }
  
  Drupal.theme.prototype.rdfPredicatesWidget = function(context, index) {
    var wrapper = $(context);
    var basepath = Drupal.settings.rdfui.basepath;
    return '<div id="" class="form-item predicates-widget">' +
      '<div class="predicate-holder"></div>' +
      '<input type="text" class="predicate-entry form-autocomplete" size="30" id="rdfui-predicate-edit-'+ index +'" />' +
      '<input type="submit" value="' + Drupal.t('Add') + '" class="form-submit predicate-add" id="rdfui-predicate-edit-button-'+ index +'">' +
      '<input class="autocomplete" type="hidden" id="rdfui-predicate-edit-'+ index +'-autocomplete" ' +
      'value="' + basepath + 'rdfui/predicates/autocomplete" disabled="disabled" />' +
      '<div class="description"></div>' +
    '</div>';
  };
  
  Drupal.theme.prototype.rdfPredicatesChangedWarning = function() {
    return '<div class="warning">* ' + Drupal.t('Changes made will not be saved until the form is submitted.') + '</div>'  
  }
 

})(jQuery);
