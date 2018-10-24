#include "WebPortal/WebPortal/WebPortalSession.hpp"

using namespace Beam;
using namespace Beam::WebServices;
using namespace Nexus;
using namespace Nexus::WebPortal;
using namespace std;

WebPortalSession::WebPortalSession(string id)
    : AuthenticatedSession{std::move(id)} {}
