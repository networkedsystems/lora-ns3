# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

import os.path

def build(bld):
	obj = bld.create_ns3_module('lora', ['core', 'network', 'mobility', 'spectrum', 'propagation', 'energy'])
	obj.source = [
	'helper/lora-helper.cc',
	'helper/lora-energy-source-helper.cc',
	'helper/lora-radio-energy-model-helper.cc',
	'model/lora-error-model.cc',
	'model/lora-radio-energy-model.cc',
	'model/lora-phy.cc',
	'model/lora-application.cc',
	'model/lora-sink-application.cc',
	'model/lora-phy-gw.cc',
	'model/lora-phy-header.cc',
	'model/lora-mac-trailer.cc',
	'model/lora-spectrum-signal-parameters.cc',
	'model/lora-mac-header.cc',
	'model/lora-mac-command.cc',
	'model/lora-net-device.cc',
	'model/lora-tsch-net-device.cc',
	'model/lora-tsch-net-device-gw.cc',
	'model/lora-net-device-gw.cc',
	'model/lora-network.cc',
	'model/lora-network-application.cc',
	'model/lora-power-application.cc',
	'model/lora-no-power-application.cc',
	'model/commands/link-check-req.cc',
	'model/commands/link-adr-ans.cc',
#	'model/commands/dev-status-ans.cc',
#	'model/commands/duty-cycle-ans.cc',
#	'model/commands/rx-param-setup-ans.cc',
#	'model/commands/new-channel-ans.cc',
#	'model/commands/rx-timing-setup-ans.cc',
	'model/commands/link-check-ans.cc',
	'model/commands/link-adr-req.cc',
#	'model/commands/dev-status-req.cc',
#	'model/commands/duty-cycle-req.cc',
#	'model/commands/rx-param-setup-req.cc',
#	'model/commands/new-channel-req.cc',
#	'model/commands/rx-timing-setup-req.cc'
	'model/gw-trailer.cc'
	]

	obj.cxxflags=['-finstrument-functions']

	module_test = bld.create_ns3_module_test_library('lora')
	module_test.source = [
	]

	headers = bld(features='ns3header')
	headers.module = 'lora'
	headers.source = [
		'helper/lora-helper.h',
		'helper/lora-energy-source-helper.h',
		'helper/lora-radio-energy-model-helper.h',
		'model/lora-error-model.h',
		'model/lora-radio-energy-model.h',
		'model/lora-phy.h',
		'model/lora-application.h',
		'model/lora-sink-application.h',
		'model/lora-phy-gw.h',
		'model/lora-phy-header.h',
		'model/lora-spectrum-signal-parameters.h',
		'model/lora-mac-header.h',
		'model/lora-mac-command.h',
		'model/lora-mac-trailer.h',
		'model/lora-net-device.h',
		'model/lora-tsch-net-device.h',
		'model/lora-tsch-net-device-gw.h',
		'model/lora-net-device-gw.h',
		'model/lora-network.h',
		'model/lora-network-application.h',
		'model/lora-power-application.h',
		'model/lora-no-power-application.h',
		'model/commands/link-check-req.h',
		'model/commands/link-adr-ans.h',
#	'model/commands/dev-status-ans.h',
#	'model/commands/duty-cycle-ans.h',
#	'model/commands/rx-param-setup-ans.h',
#	'model/commands/new-channel-ans.h',
#	'model/commands/rx-timing-setup-ans.h',
		'model/commands/link-check-ans.h',
		'model/commands/link-adr-req.h',
#	'model/commands/dev-status-req.h',
#	'model/commands/duty-cycle-req.h',
#	'model/commands/rx-param-setup-req.h',
#	'model/commands/new-channel-req.h',
#	'model/commands/rx-timing-setup-req.h'
		'model/gw-trailer.h'
		]

#  if (bld.env['ENABLE_EXAMPLES']):
#      bld.recurse('examples')

	bld.ns3_python_bindings()

