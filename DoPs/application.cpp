#include "application.h"
#include "gpk_bitmap_file.h"
#include "gpk_tcpip.h"

//#define GPK_AVOID_LOCAL_APPLICATION_MODULE_MODEL_EXECUTABLE_RUNTIME
#include "gpk_app_impl.h"
#include "gpk_endpoint_command.h"
#include "gpk_view_stream.h"

GPK_DEFINE_APPLICATION_ENTRY_POINT(::gme::SApplication, "Module Explorer");

			int														serverShutdown				(::gme::SServer& server)						{
	server.Listening													= false;
	char																	commandbytes	[256]		= {};
	::gpk::view_stream<char>												commandToSend				= {commandbytes};
	::gpk::SEndpointCommand													command						= {::gpk::ENDPOINT_COMMAND_DISCONNECT, ::gpk::ENDPOINT_MESSAGE_TYPE_REQUEST};
	commandToSend.write_pod(command);

	// Set family and port */
	::gpk::SIPv4															& local						= server.Address;
	struct sockaddr_in														sa_remote					= {};			/* Information about the server */
	sa_remote.sin_family												= AF_INET;
	sa_remote.sin_port													= htons(local.Port);
	sa_remote.sin_addr.S_un.S_un_b.s_b1									= (unsigned char)local.IP[0];
	sa_remote.sin_addr.S_un.S_un_b.s_b2									= (unsigned char)local.IP[1];
	sa_remote.sin_addr.S_un.S_un_b.s_b3									= (unsigned char)local.IP[2];
	sa_remote.sin_addr.S_un.S_un_b.s_b4									= (unsigned char)local.IP[3];
	while(server.Running) {
		warn_if(sendto(server.Socket, commandToSend.begin(), commandToSend.CursorPosition, 0, (struct sockaddr *)&sa_remote, (int)sizeof(struct sockaddr_in)) != (int32_t)commandToSend.CursorPosition, "Error sending datagram.\n");
		::gpk::sleep(10);
	}
	return 0;
}

			int														serverListen				(::gme::SServer& server);
			::gpk::error_t											cleanup						(::gme::SApplication & app)						{ 
	::serverShutdown(app.Server);
	while(app.Server.Running)
		::gpk::sleep(10);
	::gpk::tcpipShutdown();
	return ::gpk::mainWindowDestroy(app.Framework.MainDisplay); 
}

			::gpk::error_t											setup						(::gme::SApplication & app)						{ 
	::gpk::SFramework														& framework					= app.Framework;
	::gpk::SDisplay															& mainWindow				= framework.MainDisplay;
	framework.Input.create();
	error_if(errored(::gpk::mainWindowCreate(mainWindow, framework.RuntimeValues.PlatformDetail, framework.Input)), "Failed to create main window why?????!?!?!?!?");
	::gpk::SGUI																& gui						= framework.GUI;
	gui.ColorModeDefault												= ::gpk::GUI_COLOR_MODE_3D;
	gui.ThemeDefault													= ::gpk::ASCII_COLOR_DARKGREEN * 16 + 7;
	app.IdExit															= ::gpk::controlCreate(gui);
	::gpk::SControl															& controlExit				= gui.Controls.Controls[app.IdExit];
	controlExit.Area													= {{}, {64, 20}};
	controlExit.Border													= {10, 10, 10, 10};
	controlExit.Margin													= {1, 1, 1, 1};
	controlExit.Align													= ::gpk::ALIGN_BOTTOM_RIGHT;
	::gpk::SControlText														& controlText				= gui.Controls.Text[app.IdExit];
	controlText.Text													= "Exit";
	controlText.Align													= ::gpk::ALIGN_CENTER;
	::gpk::SControlConstraints												& controlConstraints		= gui.Controls.Constraints[app.IdExit];
	controlConstraints.AttachSizeToControl								= {app.IdExit, -1};
	::gpk::controlSetParent(gui, app.IdExit, -1);
	::gpk::tcpipInitialize();
	::gpk::SIPv4															addressClient				= {};
	::gpk::SIPv4															& addressServer				= app.Server.Address	= {{192, 168, 1, 79}, 6667,};
	::gpk::tcpipAddress(addressServer.Port, 0, ::gpk::TRANSPORT_PROTOCOL_UDP, addressServer);
	::gpk::tcpipAddress(addressServer.Port, 0, ::gpk::TRANSPORT_PROTOCOL_UDP, addressClient);
	serverListen(app.Server);
	return 0; 
}

			::gpk::error_t											draw					(::gme::SApplication & app)						{ 
	//::gpk::STimer															timer;
	app;
	::gpk::ptr_obj<::gpk::SRenderTarget<::gpk::SColorBGRA, uint32_t>>		target;
	target.create();
	target->Color		.resize(app.Framework.MainDisplay.Size);
	target->DepthStencil.resize(target->Color.View.metrics());
	//::gpk::clearTarget(*target);
	{
		::gme::mutex_guard														lock					(app.LockGUI);
		::gpk::controlDrawHierarchy(app.Framework.GUI, 0, target->Color.View);
	}
	{
		::gme::mutex_guard														lock					(app.LockRender);
		app.Offscreen														= target;
	}
	//timer.Frame();
	//warning_printf("Draw time: %f.", (float)timer.LastTimeSeconds);
	return 0; 
}

			::gpk::error_t											update						(::gme::SApplication & app, bool exitSignal)	{ 
	//::gpk::STimer															timer;
	retval_info_if(::gpk::APPLICATION_STATE_EXIT, exitSignal, "Exit requested by runtime.");
	{
		::gme::mutex_guard														lock						(app.LockRender);
		app.Framework.MainDisplayOffscreen									= app.Offscreen;
	}
	::gpk::SFramework														& framework					= app.Framework;
	retval_info_if(::gpk::APPLICATION_STATE_EXIT, ::gpk::APPLICATION_STATE_EXIT == ::gpk::updateFramework(app.Framework), "Exit requested by framework update.");

	::gpk::SGUI																& gui						= framework.GUI;
	{
		::gme::mutex_guard														lock						(app.LockGUI);
		::gpk::guiProcessInput(gui, *app.Framework.Input);
	}
	if(app.Framework.Input->MouseCurrent.Deltas.z) {
		gui.Zoom.ZoomLevel													+= app.Framework.Input->MouseCurrent.Deltas.z * (1.0 / (120 * 4ULL));
		::gpk::guiUpdateMetrics(gui, framework.MainDisplay.Size, true);
	}
 
	for(uint32_t iControl = 0, countControls = gui.Controls.Controls.size(); iControl < countControls; ++iControl) {
		const ::gpk::SControlState												& controlState				= gui.Controls.States[iControl];
		if(controlState.Unused || controlState.Disabled)
			continue;
		if(controlState.Execute) {
			info_printf("Executed %u.", iControl);
			if(iControl == (uint32_t)app.IdExit)
				return 1;
		}
	}
	//timer.Frame();
	//warning_printf("Update time: %f.", (float)timer.LastTimeSeconds);
	return 0; 
}
