;GIL-3
(
(
	:type                   :menu
	:name                   menu_close_profile
	:help                   ""
	:columns                1
	:menu-type              :command
	:menu-handler           action_menu_close_profile
	:menu-title             "Display Profiles"
	:menu-item-labels       ("None" )
	:menu-item-label-types  (:string )
	:menu-item-states       (:active )
	:menu-item-defaults     (nil )
	:initial-selections     (nil )
	:menu-item-handlers     (nil )
	:menu-item-menus        (nil )
	:menu-item-colors       ("" )
	:pinnable               t
	:user-data              ()
	:actions                (
		(
		:from                   (menu_close_profile)
		:when                   (Notify )
		:to                     (menu_close_profile)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (action_menu_close_profile)
		)
	)
)
(
	:type                   :menu
	:name                   menu_file
	:help                   ""
	:columns                1
	:menu-type              :command
	:menu-handler           nil
	:menu-title             "File"
	:menu-item-labels       ("Open display profile" "Open editable profile" "New editable profile" "Save editable profile" "Close display profile" "--------" "Open Multibeam data" )
	:menu-item-label-types  (:string :string :string :string :string :string :string )
	:menu-item-states       (:active :active :active :active :active :inactive :active )
	:menu-item-defaults     (nil nil nil nil nil nil nil )
	:initial-selections     (nil nil nil nil nil nil nil )
	:menu-item-handlers     (nil nil action_new_profile nil nil nil nil )
	:menu-item-menus        (nil nil nil nil menu_close_profile nil nil )
	:menu-item-colors       ("" "" "" "" "" "" "" )
	:pinnable               t
	:user-data              ()
	:actions                (
		(
		:from                   (menu_file "New editable profile")
		:when                   (Notify )
		:to                     (menu_file "New editable profile")
		:function_type          CallFunction
		:arg_type               ()
		:action                 (action_new_profile)
		)
		(
		:from                   (menu_file "Open display profile")
		:when                   (Notify )
		:to                     (popup_open_file)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Show)
		)
		(
		:from                   (menu_file "Open editable profile")
		:when                   (Notify )
		:to                     (popup_open_file)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Show)
		)
		(
		:from                   (menu_file "Open Multibeam data")
		:when                   (Notify )
		:to                     (popup_open_file)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Show)
		)
		(
		:from                   (menu_file "Open display profile")
		:when                   (Notify )
		:to                     (popup_open_file textfield_mbformat)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
		(
		:from                   (menu_file "Open editable profile")
		:when                   (Notify )
		:to                     (popup_open_file textfield_mbformat)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
		(
		:from                   (menu_file "Open Multibeam data")
		:when                   (Notify )
		:to                     (popup_open_file textfield_mbformat)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Show)
		)
		(
		:from                   (menu_file "Save editable profile")
		:when                   (Notify )
		:to                     (popup_save_file)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Show)
		)
	)
)
(
	:type                   :base-window
	:name                   window_base
	:owner                  nil
	:width                  744
	:height                 844
	:background-color       ""
	:foreground-color       ""
	:label                  "Multibeam Water Velocity Tool"
	:label-type             :string
	:initial-state          :open
	:show-footer            t
	:resizable              t
	:icon-file              ""
	:icon-label             ""
	:icon-mask-file         ""
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :control-area
	:name                   controls_base
	:owner                  window_base
	:help                   ""
	:x                      0
	:y                      0
	:width                  744
	:height                 94
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:show-border            nil
	:menu                   nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   message_status_display
	:owner                  controls_base
	:help                   ""
	:x                      8
	:y                      8
	:width                  281
	:height                 13
	:foreground-color       ""
	:label                  "No display water velocity profiles loaded"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :slider
	:name                   slider_maxdepth
	:owner                  controls_base
	:help                   ""
	:x                      376
	:y                      8
	:width                  355
	:height                 20
	:value-x                494
	:value-y                8
	:slider-width           100
	:ticks                  0
	:foreground-color       ""
	:label                  "Maximum Depth:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             t
	:show-value             t
	:min-value              250
	:max-value              13000
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :active
	:notify-handler         action_maxdepth
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window_base slider_maxdepth)
		:when                   (Notify )
		:to                     (window_base slider_maxdepth)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (action_maxdepth)
		)
	)
)
(
	:type                   :message
	:name                   message_status_edit
	:owner                  controls_base
	:help                   ""
	:x                      8
	:y                      24
	:width                  278
	:height                 13
	:foreground-color       ""
	:label                  "No editable water velocity profile loaded"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :slider
	:name                   slider_velrange
	:owner                  controls_base
	:help                   ""
	:x                      384
	:y                      32
	:width                  320
	:height                 20
	:value-x                491
	:value-y                32
	:slider-width           100
	:ticks                  0
	:foreground-color       ""
	:label                  "Velocity Range:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             t
	:show-value             t
	:min-value              25
	:max-value              1000
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :active
	:notify-handler         action_velrange
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window_base slider_velrange)
		:when                   (Notify )
		:to                     (window_base slider_velrange)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (action_velrange)
		)
	)
)
(
	:type                   :message
	:name                   message_status_hydrosweep
	:owner                  controls_base
	:help                   ""
	:x                      8
	:y                      40
	:width                  179
	:height                 13
	:foreground-color       ""
	:label                  "No multibeam data loaded"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :slider
	:name                   slider_residual_range
	:owner                  controls_base
	:help                   ""
	:x                      376
	:y                      56
	:width                  301
	:height                 20
	:value-x                488
	:value-y                56
	:slider-width           100
	:ticks                  0
	:foreground-color       ""
	:label                  "Residual Range:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             t
	:show-value             t
	:min-value              1
	:max-value              100
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          50
	:initial-state          :active
	:notify-handler         action_residual_range
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window_base slider_residual_range)
		:when                   (Notify )
		:to                     (window_base slider_residual_range)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (action_residual_range)
		)
	)
)
(
	:type                   :button
	:name                   button_file
	:owner                  controls_base
	:help                   ""
	:x                      24
	:y                      64
	:width                  54
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "File"
	:label-type             :string
	:initial-state          :active
	:menu                   menu_file
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   button_process_hydrosweep
	:owner                  controls_base
	:help                   ""
	:x                      104
	:y                      64
	:width                  142
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Process Multibeam"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         action_process_hydrsoweep
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window_base button_process_hydrosweep)
		:when                   (Notify )
		:to                     (window_base button_process_hydrosweep)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (action_process_hydrsoweep)
		)
	)
)
(
	:type                   :button
	:name                   button_quit
	:owner                  controls_base
	:help                   ""
	:x                      288
	:y                      64
	:width                  45
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Quit"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         action_quit
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window_base button_quit)
		:when                   (Notify )
		:to                     (window_base button_quit)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (action_quit)
		)
	)
)
(
	:type                   :canvas-pane
	:name                   canvas_base
	:owner                  window_base
	:help                   ""
	:x                      0
	:y                      94
	:width                  744
	:height                 750
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       744
	:vertical-scrollbar     nil
	:scrollable-height      750
	:repaint-proc           nil
	:event-handler          action_canvas_event
	:drawing-model          :xview
	:user-data              ()
	:actions                (
		(
		:from                   (window_base canvas_base)
		:when                   (AnyEvent )
		:to                     (window_base canvas_base)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (action_canvas_event)
		)
	)
)
(
	:type                   :popup-window
	:name                   popup_open_file
	:owner                  window_base
	:width                  237
	:height                 395
	:background-color       ""
	:foreground-color       ""
	:label                  "Open File"
	:label-type             :string
	:initial-state          :invisible
	:show-footer            t
	:resizable              t
	:pinned                 nil
	:done-handler           nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :control-area
	:name                   controls_open_file
	:owner                  popup_open_file
	:help                   ""
	:x                      0
	:y                      0
	:width                  237
	:height                 395
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:show-border            nil
	:menu                   nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :scrolling-list
	:name                   list_open_file
	:owner                  controls_open_file
	:help                   ""
	:x                      8
	:y                      8
	:width                  200
	:height                 314
	:value-x                8
	:value-y                8
	:rows                   15
	:foreground-color       ""
	:label                  ""
	:title                  "Available FIles:"
	:label-type             :string
	:layout-type            :horizontal
	:read-only              nil
	:multiple-selections    nil
	:selection-required     nil
	:initial-state          :active
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:initial-list-values    ()
	:initial-list-glyphs    ()
	:initial-selections     ()
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   textfield_mbformat
	:owner                  controls_open_file
	:help                   ""
	:x                      16
	:y                      336
	:width                  187
	:height                 15
	:value-x                152
	:value-y                336
	:value-length           2
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Multibeam Format:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100
	:min-value              0
	:initial-value          1
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   button_open_file_ok
	:owner                  controls_open_file
	:help                   ""
	:x                      24
	:y                      360
	:width                  36
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "OK"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (popup_open_file button_open_file_ok)
		:when                   (Notify )
		:to                     (popup_open_file)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
	)
)
(
	:type                   :button
	:name                   button_open_file_cancel
	:owner                  controls_open_file
	:help                   ""
	:x                      88
	:y                      360
	:width                  59
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Cancel"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (popup_open_file button_open_file_cancel)
		:when                   (Notify )
		:to                     (popup_open_file)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
	)
)
(
	:type                   :popup-window
	:name                   popup_save_file
	:owner                  window_base
	:width                  449
	:height                 103
	:background-color       ""
	:foreground-color       ""
	:label                  "Save File"
	:label-type             :string
	:initial-state          :invisible
	:show-footer            t
	:resizable              t
	:pinned                 nil
	:done-handler           nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :control-area
	:name                   controls_save_file
	:owner                  popup_save_file
	:help                   ""
	:x                      0
	:y                      0
	:width                  449
	:height                 103
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:show-border            nil
	:menu                   nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   textfield_save_file
	:owner                  controls_save_file
	:help                   ""
	:x                      24
	:y                      16
	:width                  400
	:height                 32
	:value-x                24
	:value-y                33
	:value-length           50
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Text Field:"
	:label-type             :string
	:layout-type            :vertical
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   button_save_file_ok
	:owner                  controls_save_file
	:help                   ""
	:x                      32
	:y                      64
	:width                  36
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "OK"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (popup_save_file button_save_file_ok)
		:when                   (Notify )
		:to                     (popup_save_file)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
	)
)
(
	:type                   :button
	:name                   button_save_file_cancel
	:owner                  controls_save_file
	:help                   ""
	:x                      128
	:y                      64
	:width                  59
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Cancel"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (popup_save_file button_save_file_cancel)
		:when                   (Notify )
		:to                     (popup_save_file)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
	)
)
)
