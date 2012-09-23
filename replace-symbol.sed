#! /bin/sed -f
s/g_signal_connect_data/fph_g_signal_connect_/g
s/gtk_widget_grab_focus/fph_gtk_widget_grab_f/g
s@libgtk-x11-2.0.so.0@/opt/fph/libfph.so\x00@g
