;GIL-3
(
(
	:type                   :menu
	:name                   menu_file
	:help                   ""
	:columns                1
	:menu-type              :command
	:menu-handler           nil
	:menu-title             "File"
	:menu-item-labels       ("Load" "Show Colors" )
	:menu-item-label-types  (:string :string )
	:menu-item-states       (:active :active )
	:menu-item-defaults     (nil nil )
	:initial-selections     (nil nil )
	:menu-item-handlers     (nil action_show_colors )
	:menu-item-menus        (nil nil )
	:menu-item-colors       ("" "" )
	:pinnable               t
	:user-data              ()
	:actions                (
		(
		:from                   (menu_file "Load")
		:when                   (Notify )
		:to                     (popup_load)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Show)
		)
		(
		:from                   (menu_file "Show Colors")
		:when                   (Notify )
		:to                     (menu_file "Show Colors")
		:function_type          CallFunction
		:arg_type               ()
		:action                 (action_show_colors)
		)
	)
)
(
	:type                   :base-window
	:name                   window_mbedit
	:owner                  nil
	:width                  897
	:height                 764
	:background-color       ""
	:foreground-color       ""
	:label                  "MBEDIT Version $Id: mbedit.G,v 3.1 1993-08-17 04:32:17 caress Exp $"
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
	:name                   controls_mbedit
	:owner                  window_mbedit
	:help                   ""
	:x                      0
	:y                      0
	:width                  897
	:height                 107
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
	:type                   :button
	:name                   button_file
	:owner                  controls_mbedit
	:help                   ""
	:x                      8
	:y                      8
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
	:name                   button_quit
	:owner                  controls_mbedit
	:help                   ""
	:x                      104
	:y                      8
	:width                  45
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Quit"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         do_quit
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window_mbedit button_quit)
		:when                   (Notify )
		:to                     (window_mbedit button_quit)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (do_quit)
		)
	)
)
(
	:type                   :slider
	:name                   slider_number_pings
	:owner                  controls_mbedit
	:help                   ""
	:x                      160
	:y                      8
	:width                  364
	:height                 20
	:value-x                331
	:value-y                8
	:slider-width           100
	:ticks                  0
	:foreground-color       ""
	:label                  "Number of pings shown:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             t
	:show-value             t
	:min-value              1
	:max-value              20
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          10
	:initial-state          :active
	:notify-handler         set_number_pings
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window_mbedit slider_number_pings)
		:when                   (Notify )
		:to                     (window_mbedit slider_number_pings)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_number_pings)
		)
	)
)
(
	:type                   :text-field
	:name                   textfield_x_interval
	:owner                  controls_mbedit
	:help                   ""
	:x                      560
	:y                      8
	:width                  246
	:height                 15
	:value-x                731
	:value-y                8
	:value-length           5
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "X Axis Tick Interval (m):"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         do_x_interval
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window_mbedit textfield_x_interval)
		:when                   (Notify )
		:to                     (window_mbedit textfield_x_interval)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (do_x_interval)
		)
	)
)
(
	:type                   :slider
	:name                   slider_scale_x
	:owner                  controls_mbedit
	:help                   ""
	:x                      160
	:y                      32
	:width                  380
	:height                 20
	:value-x                319
	:value-y                32
	:slider-width           100
	:ticks                  0
	:foreground-color       ""
	:label                  "X Scale (m/100 pixels):"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             t
	:show-value             t
	:min-value              1
	:max-value              10000
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          1000
	:initial-state          :active
	:notify-handler         set_scale_x
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window_mbedit slider_scale_x)
		:when                   (Notify )
		:to                     (window_mbedit slider_scale_x)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_scale_x)
		)
	)
)
(
	:type                   :text-field
	:name                   textfield_y_interval
	:owner                  controls_mbedit
	:help                   ""
	:x                      560
	:y                      32
	:width                  246
	:height                 15
	:value-x                731
	:value-y                32
	:value-length           5
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Y Axis Tick Interval (m):"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         do_y_interval
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window_mbedit textfield_y_interval)
		:when                   (Notify )
		:to                     (window_mbedit textfield_y_interval)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (do_y_interval)
		)
	)
)
(
	:type                   :button
	:name                   button_next_buffer
	:owner                  controls_mbedit
	:help                   ""
	:x                      8
	:y                      40
	:width                  87
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Next Buffer"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         do_next_buffer
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window_mbedit button_next_buffer)
		:when                   (Notify )
		:to                     (window_mbedit button_next_buffer)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (do_next_buffer)
		)
	)
)
(
	:type                   :button
	:name                   button_done
	:owner                  controls_mbedit
	:help                   ""
	:x                      104
	:y                      40
	:width                  49
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Done"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         do_done
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window_mbedit button_done)
		:when                   (Notify )
		:to                     (window_mbedit button_done)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (do_done)
		)
	)
)
(
	:type                   :slider
	:name                   slider_scale_y
	:owner                  controls_mbedit
	:help                   ""
	:x                      160
	:y                      56
	:width                  380
	:height                 20
	:value-x                319
	:value-y                56
	:slider-width           100
	:ticks                  0
	:foreground-color       ""
	:label                  "Y Scale (m/100 pixels):"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             t
	:show-value             t
	:min-value              1
	:max-value              10000
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          1000
	:initial-state          :active
	:notify-handler         set_scale_y
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window_mbedit slider_scale_y)
		:when                   (Notify )
		:to                     (window_mbedit slider_scale_y)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_scale_y)
		)
	)
)
(
	:type                   :slider
	:name                   slider_buffer_size
	:owner                  controls_mbedit
	:help                   ""
	:x                      560
	:y                      56
	:width                  330
	:height                 20
	:value-x                689
	:value-y                56
	:slider-width           100
	:ticks                  0
	:foreground-color       ""
	:label                  "Data Buffer Size:   "
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             t
	:show-value             t
	:min-value              0
	:max-value              100
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :active
	:notify-handler         do_buffer_size
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window_mbedit slider_buffer_size)
		:when                   (Notify )
		:to                     (window_mbedit slider_buffer_size)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (do_buffer_size)
		)
	)
)
(
	:type                   :button
	:name                   button_forward
	:owner                  controls_mbedit
	:help                   ""
	:x                      8
	:y                      80
	:width                  68
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Forward"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         do_forward
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window_mbedit button_forward)
		:when                   (Notify )
		:to                     (window_mbedit button_forward)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (do_forward)
		)
	)
)
(
	:type                   :button
	:name                   button_reverse
	:owner                  controls_mbedit
	:help                   ""
	:x                      88
	:y                      80
	:width                  65
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Reverse"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         do_reverse
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window_mbedit button_reverse)
		:when                   (Notify )
		:to                     (window_mbedit button_reverse)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (do_reverse)
		)
	)
)
(
	:type                   :text-field
	:name                   textfield_number_step
	:owner                  controls_mbedit
	:help                   ""
	:x                      160
	:y                      80
	:width                  249
	:height                 15
	:value-x                338
	:value-y                80
	:value-length           3
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Number of pings to step:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              20
	:min-value              1
	:initial-value          5
	:initial-state          :active
	:read-only              nil
	:notify-handler         set_number_step
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window_mbedit textfield_number_step)
		:when                   (Notify )
		:to                     (window_mbedit textfield_number_step)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_number_step)
		)
	)
)
(
	:type                   :slider
	:name                   slider_buffer_hold
	:owner                  controls_mbedit
	:help                   ""
	:x                      560
	:y                      80
	:width                  331
	:height                 20
	:value-x                690
	:value-y                80
	:slider-width           100
	:ticks                  0
	:foreground-color       ""
	:label                  "Buffer Retain Size:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             t
	:show-value             t
	:min-value              0
	:max-value              100
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :active
	:notify-handler         do_buffer_hold
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window_mbedit slider_buffer_hold)
		:when                   (Notify )
		:to                     (window_mbedit slider_buffer_hold)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (do_buffer_hold)
		)
	)
)
(
	:type                   :canvas-pane
	:name                   canvas_mbedit
	:owner                  window_mbedit
	:help                   ""
	:x                      0
	:y                      107
	:width                  897
	:height                 657
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       897
	:vertical-scrollbar     nil
	:scrollable-height      657
	:repaint-proc           nil
	:event-handler          do_event
	:drawing-model          :xview
	:user-data              ()
	:actions                (
		(
		:from                   (window_mbedit canvas_mbedit)
		:when                   (AnyEvent )
		:to                     (window_mbedit canvas_mbedit)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (do_event)
		)
	)
)
(
	:type                   :popup-window
	:name                   popup_load
	:owner                  window_mbedit
	:width                  267
	:height                 375
	:background-color       ""
	:foreground-color       ""
	:label                  "Load Input Multibeam File"
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
	:name                   controls_load
	:owner                  popup_load
	:help                   ""
	:x                      0
	:y                      0
	:width                  267
	:height                 375
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
	:name                   list_files
	:owner                  controls_load
	:help                   ""
	:x                      16
	:y                      16
	:width                  220
	:height                 224
	:value-x                16
	:value-y                33
	:rows                   10
	:foreground-color       ""
	:label                  ""
	:title                  "Set Input Multibeam File:"
	:label-type             :string
	:layout-type            :vertical
	:read-only              nil
	:multiple-selections    nil
	:selection-required     nil
	:initial-state          :active
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:notify-handler         set_output_name
	:event-handler          nil
	:initial-list-values    ()
	:initial-list-glyphs    ()
	:initial-selections     ()
	:user-data              ()
	:actions                (
		(
		:from                   (popup_load list_files)
		:when                   (Notify )
		:to                     (popup_load list_files)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_output_name)
		)
	)
)
(
	:type                   :text-field
	:name                   textfield_format
	:owner                  controls_load
	:help                   ""
	:x                      16
	:y                      256
	:width                  189
	:height                 15
	:value-x                134
	:value-y                256
	:value-length           2
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "MBIO Format ID:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              20
	:min-value              1
	:initial-value          1
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   textfield_output
	:owner                  controls_load
	:help                   ""
	:x                      16
	:y                      280
	:width                  232
	:height                 32
	:value-x                16
	:value-y                297
	:value-length           29
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Output File:"
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
	:name                   button_load_cancel
	:owner                  controls_load
	:help                   ""
	:x                      32
	:y                      336
	:width                  59
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Cancel"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         do_load_cancel
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (popup_load button_load_cancel)
		:when                   (Notify )
		:to                     (popup_load button_load_cancel)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (do_load_cancel)
		)
		(
		:from                   (popup_load button_load_cancel)
		:when                   (Notify )
		:to                     (popup_load)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
	)
)
(
	:type                   :button
	:name                   button_load_ok
	:owner                  controls_load
	:help                   ""
	:x                      192
	:y                      336
	:width                  36
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "OK"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         do_load_ok
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (popup_load button_load_ok)
		:when                   (Notify )
		:to                     (popup_load button_load_ok)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (do_load_ok)
		)
		(
		:from                   (popup_load button_load_ok)
		:when                   (Notify )
		:to                     (popup_load)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
	)
)
)
