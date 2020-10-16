#pragma once
struct SaveData
{
	int center_x;
	int center_y;
	double u1_pulse_pixel;
	double u3_pulse_pixel;
	double u1_u2_scale;
	int cols_mat_pulse;
	int row_mat_pulse;
	int space_cols_pulse;
	int space_rows_pulse;
	int camera_start_pulse;
	int bottom_jacking_pulse;
	int vacuumpads_camera_pulse;
	int first_mat_pulse;
	int second_mat_pulse;
	int valid_mat_cols;
	int postion_mat;

	int opencv_thread;
	int opencv_template_circle;

	int b_save_data;
	int save_mat_pulse;
	int save_jacking_pulse;
	int save_surplus_cols;
	int save_cols6;
	int save_cols5;
	 
};


struct Golbal_Bool_State {
	bool open_device_single;
	bool image_show_start;
	bool image_shoe_stop;

	bool material_send;
	bool socket_send_stop;
	bool socket_air_sucess;
	bool socket_connect;
	bool record_valid;

	bool runing_state;
	bool opencv_show;

	bool even_num;
	bool reset_first;

	bool force_quit;
	bool start_push;
	bool host_connect;
};

