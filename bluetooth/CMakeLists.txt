add_library(bluetooth)
target_sources(bluetooth PRIVATE
	acl.c
	assert.c
	csr.c
	fifo.c
	hci_3wire.c
	hci_core.c
	hci_event.c
	hci_h4.c
	l2cap.c
	pktf.c
	rdabt.c
	rtlbt.c
	user_cb.c
	sbc_codec.c
	)

target_link_libraries(bluetooth PUBLIC soc sbc)
