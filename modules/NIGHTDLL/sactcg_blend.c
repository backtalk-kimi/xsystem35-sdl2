// 
static surface_t *blend(surface_t *base, int x, int y, surface_t *blend, int mode) {
	surface_t *dst = sf_create_surface(base->width, base->height, base->depth);
	
	gr_copy(dst, 0, 0, base, 0, 0, base->width, base->height);
	
	if (base->has_alpha) {
		// �١����� alpha map ��������Ϥ���򥳥ԡ�
		gr_copy_alpha_map(dst, 0, 0, base, 0, 0, base->width, base->height);
	} else {
		// ̵���������� 255 �� map �����
		gr_fill_alpha_map(dst, 0, 0, base->width, base->height, 255);
	}
	
	if (blend->has_alpha) {
		// �Ť͹�碌��� alpha map ������Ȥ��Ϥ����Ȥ�
		gre_BlendUseAMap(dst, x, y, base, x, y, blend, 0, 0, blend->width, blend->height, blend, 0, 0, 255);
	} else {
		// ̵���Ȥ��� dst �� alpha map ��Ȥ�
		gre_BlendUseAMap(dst, x, y, base, x, y, blend, 0, 0, blend->width, blend->height, dst, x, y, 255);
	}
	
	// alpha �����⡼�ɤ� dst + blend �λ���˰�²û�
	if (mode == 1) {
		gr_saturadd_alpha_map(dst, x, y, blend, 0, 0, blend->width, blend->height);
	}
	
	return dst;
}


