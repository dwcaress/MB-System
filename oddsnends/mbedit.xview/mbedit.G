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
		:from                   (menu_file "Show Colors")
		:when                   (Notify )
		:to                     (menu_file "Show Colors")
		:function_type          CallFunction
		:arg_type               ()
		:action                 (action_show_colors)
		)
		(
		:from                   (menu_file "Load")
		:when                   (Notify )
		:to                     (popup_load)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Show)
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
	:label                  "MBEDIT Version $Id: mbedit.G,v 4.0 1994-03-05 23:54:35 caress Exp $"
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
	:height                 134
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
	:name                   slider_scale_x
	:owner                  controls_mbedit
	:help                   ""
	:x                      160
	:y                      8
	:width                  380
	:height                 20
	:value-x                324
	:value-y                8
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
	:max-value              50000
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
	:name                   textfield_x_interval
	:owner                  controls_mbedit
	:help                   ""
	:x                      616
	:y                      8
	:width                  246
	:height                 15
	:value-x                787
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
	:name                   slider_scale_y
	:owner                  controls_mbedit
	:help                   ""
	:x                      160
	:y                      32
	:width                  402
	:height                 20
	:value-x                362
	:value-y                32
	:slider-width           100
	:ticks                  0
	:foreground-color       ""
	:label                  "Vertical Exageration (X 0.01):"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             t
	:show-value             t
	:min-value              1
	:max-value              5000
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          100
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
	:type                   :text-field
	:name                   textfield_y_interval
	:owner                  controls_mbedit
	:help                   ""
	:x                      616
	:y                      32
	:width                  246
	:height                 15
	:value-x                787
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
	:name                   slider_number_pings
	:owner                  controls_mbedit
	:help                   ""
	:x                      160
	:y                      56
	:width                  344
	:height                 20
	:value-x                336
	:value-y                56
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
	:type                   :slider
	:name                   slider_buffer_size
	:owner                  controls_mbedit
	:help                   ""
	:x                      552
	:y                      56
	:width                  318
	:height                 20
	:value-x                681
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
	:y                      72
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
	:y                      72
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
	:width                  237
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
	:x                      552
	:y                      80
	:width                  319
	:height                 20
	:value-x                682
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
	:type                   :setting
	:name                   setting_mode
	:owner                  controls_mbedit
	:help                   ""
	:x                      8
	:y                      104
	:width                  197
	:height                 23
	:value-x                55
	:value-y                104
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Mode:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("Pick" "Erase" "Restore" )
	:choice-label-types     (:string :string :string )
	:choice-colors          ("" "" "" )
	:initial-selections     (nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (window_mbedit setting_mode "Pick")
		:when                   (Notify )
		:to                     (window_mbedit setting_mode "Pick")
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_mode_pick)
		)
		(
		:from                   (window_mbedit setting_mode "Erase")
		:when                   (Notify )
		:to                     (window_mbedit setting_mode "Erase")
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_mode_erase)
		)
		(
		:from                   (window_mbedit setting_mode "Restore")
		:when                   (Notify )
		:to                     (window_mbedit setting_mode "Restore")
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_mode_restore)
		)
	)
)
(
	:type                   :button
	:name                   button_goto
	:owner                  controls_mbedit
	:help                   ""
	:x                      272
	:y                      104
	:width                  53
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Go To"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window_mbedit button_goto)
		:when                   (Notify )
		:to                     (popup_goto)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Show)
		)
	)
)
(
	:type                   :canvas-pane
	:name                   canvas_mbedit
	:owner                  window_mbedit
	:help                   ""
	:x                      0
	:y                      134
	:width                  897
	:height                 630
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
	:scrollable-height      630
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
	:width                  169
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
(
	:type                   :popup-window
	:name                   popup_goto
	:owner                  window_mbedit
	:width                  179
	:height                 220
	:background-color       ""
	:foreground-color       ""
	:label                  "Great Leap Forward?"
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
	:name                   controls1
	:owner                  popup_goto
	:help                   ""
	:x                      0
	:y                      0
	:width                  179
	:height                 220
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
	:name                   textfield_year
	:owner                  controls1
	:help                   ""
	:x                      24
	:y                      24
	:width                  109
	:height                 15
	:value-x                66
	:value-y                24
	:value-length           4
	:stored-length          4
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Year:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              2062
	:min-value              1962
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   textfield_month
	:owner                  controls1
	:help                   ""
	:x                      24
	:y                      48
	:width                  105
	:height                 15
	:value-x                78
	:value-y                48
	:value-length           2
	:stored-length          2
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Month:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              12
	:min-value              1
	:initial-value          2
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   textfield_day
	:owner                  controls1
	:help                   ""
	:x                      24
	:y                      72
	:width                  88
	:height                 15
	:value-x                61
	:value-y                72
	:value-length           2
	:stored-length          2
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Day:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              31
	:min-value              1
	:initial-value          21
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   textfield_hour
	:owner                  controls1
	:help                   ""
	:x                      24
	:y                      96
	:width                  95
	:height                 15
	:value-x                68
	:value-y                96
	:value-length           2
	:stored-length          2
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Hour:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              23
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   textfield_minute
	:owner                  controls1
	:help                   ""
	:x                      24
	:y                      120
	:width                  110
	:height                 15
	:value-x                83
	:value-y                120
	:value-length           2
	:stored-length          2
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Minute:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              59
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   textfield_second
	:owner                  controls1
	:help                   ""
	:x                      24
	:y                      144
	:width                  110
	:height                 15
	:value-x                83
	:value-y                144
	:value-length           2
	:stored-length          2
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Second:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              59
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   button_goto_apply
	:owner                  controls1
	:help                   ""
	:x                      56
	:y                      176
	:width                  53
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Apply"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (popup_goto button_goto_apply)
		:when                   (Notify )
		:to                     (popup_goto)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
	)
)
)
