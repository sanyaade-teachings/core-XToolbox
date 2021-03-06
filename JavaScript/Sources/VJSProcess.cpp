/*
* This file is part of Wakanda software, licensed by 4D under
*  (i) the GNU General Public License version 3 (GNU GPL v3), or
*  (ii) the Affero General Public License version 3 (AGPL v3) or
*  (iii) a commercial license.
* This file remains the exclusive property of 4D and/or its licensors
* and is protected by national and international legislations.
* In any event, Licensee's compliance with the terms and conditions
* of the applicable license constitutes a prerequisite to any use of this file.
* Except as otherwise expressly stated in the applicable license,
* such license does not include any other license or rights on this file,
* 4D's and/or its licensors' trademarks and/or other proprietary rights.
* Consequently, no title, copyright or other proprietary rights
* other than those specified in the applicable license is granted.
*/
#include "VJavaScriptPrecompiled.h"

#include <list>

#include "ServerNet/VServerNet.h"
#include "VJSProcess.h"

#include "VJSContext.h"
#include "VJSGlobalClass.h"


USING_TOOLBOX_NAMESPACE


void VJSProcess::Initialize ( const XBOX::VJSParms_initialize& inParms, void* )
{
	;
}

void VJSProcess::Finalize ( const XBOX::VJSParms_finalize& inParms, void* )
{
	;
}

void VJSProcess::GetDefinition( ClassDefinition& outDefinition)
{
	static inherited::StaticFunction functions[] =
	{
		{ 0, 0, 0}
	};

	static inherited::StaticValue values[] =
	{
		{ "version", js_getProperty<_Version>, NULL, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ "buildNumber", js_getProperty<_BuildNumber>, NULL, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ "userDocuments", js_getProperty<_UserDocumentsFolder>, NULL, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0, 0}
	};

	outDefinition.className = "process";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.staticFunctions = functions;
	outDefinition.staticValues = values;
}

void VJSProcess::_Version ( XBOX::VJSParms_getProperty& ioParms, void* )
{
	XBOX::VString		vstrVersion;
	XBOX::VProcess::Get ( )-> GetProductVersion ( vstrVersion );

	ioParms. ReturnString ( vstrVersion );
}

void VJSProcess::_BuildNumber ( XBOX::VJSParms_getProperty& ioParms, void* )
{
	VString s;
	VProcess::Get()->GetBuildNumber( s);
	ioParms.ReturnString( s);
}

void VJSProcess::_UserDocumentsFolder ( XBOX::VJSParms_getProperty& ioParms, void* )
{
	VFolder*		fldrUserDocuments = VFolder::RetainSystemFolder ( eFK_UserDocuments, false );
	ioParms. ReturnFolder ( fldrUserDocuments );
	ReleaseRefCountable ( &fldrUserDocuments );
}

void VJSProcess::_getAccessor( XBOX::VJSParms_getProperty& ioParms, XBOX::VJSGlobalObject* inGlobalObject)
{
	ioParms.ReturnValue( VJSProcess::CreateInstance( ioParms.GetContextRef(), NULL));
}

void VJSOS::GetDefinition( ClassDefinition& outDefinition)
{
	typedef XBOX::VJSClass<VJSOS, void>	inherited;

	static inherited::StaticFunction functions[] =
	{
		{	"type",					js_callStaticFunction<_Type>,				JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete	},
		{	"networkInterfaces",	js_callStaticFunction<_NetworkInterfaces>,	JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete	},
		{	"getProxy",				js_callStaticFunction<_getProxy>,			JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete	},
		{	0,						0,											0																	},
	};

	static inherited::StaticValue values[] =
	{
		{	"isWindows",			js_getProperty<_IsWindows>,			NULL,	JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{	"isMac",				js_getProperty<_IsMac>,				NULL,	JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{	"isLinux",				js_getProperty<_IsLinux>,			NULL,	JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete	},
		{	0,						0,									0,		0																	},
	};

	outDefinition.className			= "os";
	outDefinition.initialize		= js_initialize<_Initialize>;
	outDefinition.finalize			= js_finalize<_Finalize>;
	outDefinition.staticFunctions	= functions;
	outDefinition.staticValues		= values;
}

void VJSOS::_Initialize (const XBOX::VJSParms_initialize &inParms, void *)
{
}

void VJSOS::_Finalize (const XBOX::VJSParms_finalize &inParms, void *)
{
}

void VJSOS::_Type (XBOX::VJSParms_callStaticFunction &ioParms, void *)
{
	XBOX::VString	vstrOSVersion;

	XBOX::VSystem::GetOSVersionString(vstrOSVersion);
	ioParms.ReturnString(vstrOSVersion);
}

void VJSOS::_NetworkInterfaces (XBOX::VJSParms_callStaticFunction &ioParms, void *)
{
	XBOX::VNetAddressList	addressList;
	
	addressList.FromLocalInterfaces();


	std::list<Address>						addresses;
	XBOX::VNetAddressList::const_iterator	i;

	// TODO: 
	//	1) Retrieve actual interface name (currently set dummy name). 
	//	2) Support listing of both IPv4 *and* IPv6 addresses if the interface handles both.

//**
	sLONG			counter			= 0;
	XBOX::VString	interfaceName	= "InterfaceX";
//**

	for (i = addressList.begin(); i != addressList.end(); i++) {

		Address	address;

//**
		interfaceName.SetUniChar(10, '0' + counter++);
//**

		address.fInterfaceName = interfaceName;
		address.fIPAddress = i->GetIP();
		address.fIsIPv6 = i->IsV6();
		address.fIsInternal = i->IsLoopBack();

		addresses.push_back(address);
		
	}

	XBOX::VJSObject	result(ioParms.GetContext());

	result.MakeEmpty();
	if (!addresses.empty()) {

		std::list<Address>::iterator	j;

		for (j = addresses.begin(); j != addresses.end(); j++) {

			XBOX::VJSObject	entryObject(ioParms.GetContext());

			entryObject.MakeEmpty();
			entryObject.SetProperty("address", j->fIPAddress);
			entryObject.SetProperty("family", j->fIsIPv6 ? XBOX::VString("IPv6") : XBOX::VString("IPv4"));
			entryObject.SetProperty("internal", j->fIsInternal);
			
			if (result.HasProperty(j->fInterfaceName)) {

				XBOX::VJSObject	object(ioParms.GetContext());

				object = result.GetPropertyAsObject(j->fInterfaceName);
				xbox_assert(object.IsArray());

				XBOX::VJSArray	arrayObject(object);

				arrayObject.PushValue(entryObject);	

			} else {

				XBOX::VJSArray	arrayObject(ioParms.GetContext());

				result.SetProperty(j->fInterfaceName, arrayObject);
				arrayObject.PushValue(entryObject);	
				
			}

		}

	}
	ioParms.ReturnValue(result);
}


void VJSOS::_getProxy(XBOX::VJSParms_callStaticFunction &ioParms, void *)
{
	XBOX::VString pUrl;
    bool resUrl;
    resUrl=ioParms.GetStringParam(1, pUrl);

	XBOX::VURL url(pUrl, true /*is encoded*/);

	XBOX::VString scheme;
	url.GetScheme(scheme);
	scheme.ToUpperCase();

	XBOX::VString host;
	url.GetHostName(host, false /*want unencoded*/);

	bool needProxy=false;

	if(pUrl.CompareToString("HTTP", false /*not case sensitive*/)==CR_EQUAL)
		needProxy=true;

	if(!needProxy && scheme.EqualToString("HTTP", false /*not case sensitive*/)
		&& XBOX::VProxyManager::GetUseProxy()
		&& !XBOX::VProxyManager::ByPassProxyOnLocalhost(host)
		&& !XBOX::VProxyManager::MatchProxyException(host))
		needProxy=true;

	if(needProxy)
	{
		XBOX::VJSObject	result(ioParms.GetContext());

		result.MakeEmpty();

		result.SetProperty("host", XBOX::VProxyManager::GetProxyServerAddress());
		result.SetProperty("port", XBOX::VProxyManager::GetProxyServerPort());

		ioParms.ReturnValue(result);
	}
}

void VJSOS::_IsMac (XBOX::VJSParms_getProperty &ioParms, void *)
{
#if VERSIONMAC
	ioParms.ReturnBool(true);
#else
	ioParms.ReturnBool(false);
#endif
}

void VJSOS::_IsWindows (XBOX::VJSParms_getProperty &ioParms, void *)
{
#if VERSIONWIN
	ioParms.ReturnBool(true);
#else
	ioParms.ReturnBool(false);
#endif
}

void VJSOS::_IsLinux (XBOX::VJSParms_getProperty &ioParms, void *)
{
#if VERSION_LINUX
	ioParms.ReturnBool(true);
#else
	ioParms.ReturnBool(false);
#endif
}