import App from 'containers/App';
import deviceDetector from 'utils/device-detector';

var isMobile = deviceDetector.isMobile();

function errorLoading(err) {
  console.error('Dynamic page loading failed', err);
}

function loadRoute(cb) {
  return (module) => cb(null, module.default);
}

let childRoutes;

if (!isMobile) {
  // desktop routes
  childRoutes = [
    {
      path: '/',
      getComponent(location, cb) {
        System.import('components/structures/common/home')
          .then(loadRoute(cb))
          .catch(errorLoading);
      }
    },
    {
      path: 'searchProfiles',
      getComponent(location, cb) {
        System.import('components/structures/common/search-profiles')
          .then(loadRoute(cb))
          .catch(errorLoading);
      }
    }
  ];
} else {
  // mobile routes
  childRoutes = [];
}

export default {
  component: App,
  childRoutes: childRoutes
};