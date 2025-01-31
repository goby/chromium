// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @fileoverview Suite of tests for site-settings-category. */
cr.define('site_settings_category', function() {
  function registerTests() {
    suite('SiteSettingsCategory', function() {
      /**
       * A site settings category created before each test.
       * @type {SiteSettingsCategory}
       */
      var testElement;

      /**
       * The mock proxy object to use during test.
       * @type {TestSiteSettingsPrefsBrowserProxy}
       */
      var browserProxy = null;

      /**
       * An example pref where the location category is disabled.
       * @type {SiteSettingsPref}
       */
      var prefsLocationDisabled = {
        defaults: {
          geolocation: {
            setting: 'block',
          },
        },
        exceptions: {
          geolocation: [],
        },
      };

      /**
       * An example pref where the location category is enabled.
       * @type {SiteSettingsPref}
       */
      var prefsLocationEnabled = {
        defaults: {
          geolocation: {
            setting: 'allow',
          },
        },
        exceptions: {
          geolocation: [],
        },
      };

      /**
       * An example pref where the Flash category is set on detect mode.
       */
      var prefsFlashDetect = {
        defaults: {
          plugins: {
            setting: 'detect_important_content',
          },
        },
        exceptions: {
          plugins: [],
        },
      };

      /**
       * An example pref where the Cookies category is set to delete when
       * session ends.
       */
      var prefsCookesSessionOnly = {
        defaults: {
          cookies: {
            setting: 'session_only',
          },
        },
        exceptions: {
          cookies: [],
        },
      };

      // Import necessary html before running suite.
      suiteSetup(function() {
        return PolymerTest.importHtml(
           'chrome://md-settings/site_settings/site_settings_category.html');
      });

      // Initialize a site-settings-category before each test.
      setup(function() {
        browserProxy = new TestSiteSettingsPrefsBrowserProxy();
        settings.SiteSettingsPrefsBrowserProxyImpl.instance_ = browserProxy;
        PolymerTest.clearBody();
        testElement = document.createElement('site-settings-category');
        document.body.appendChild(testElement);
      });

      test('getDefaultValueForContentType API used', function() {
        testElement.category = settings.ContentSettingsTypes.GEOLOCATION;
        return browserProxy.whenCalled('getDefaultValueForContentType').then(
            function(contentType) {
              assertEquals(
                  settings.ContentSettingsTypes.GEOLOCATION, contentType);
            });
      });

      function testCategoryEnabled(testElement, enabled) {
        browserProxy.reset();
        browserProxy.setPrefs(
            enabled ? prefsLocationEnabled : prefsLocationDisabled);

        testElement.category = settings.ContentSettingsTypes.GEOLOCATION;
        return browserProxy.whenCalled('getDefaultValueForContentType').then(
          function(contentType) {
            assertEquals(
                settings.ContentSettingsTypes.GEOLOCATION, contentType);
            assertEquals(enabled, testElement.categoryEnabled);
            MockInteractions.tap(testElement.$.toggle);
            return browserProxy.whenCalled('setDefaultValueForContentType');
          }).then(function(args) {
            assertEquals(
                settings.ContentSettingsTypes.GEOLOCATION, args[0]);
            assertEquals(
                enabled ? settings.PermissionValues.BLOCK :
                    settings.PermissionValues.ASK,
                args[1]);
            assertNotEquals(enabled, testElement.categoryEnabled);
          });
      }

      test('categoryEnabled correctly represents prefs (enabled)', function() {
        return testCategoryEnabled(testElement, true);
      });

      test('categoryEnabled correctly represents prefs (disabled)', function() {
        return testCategoryEnabled(testElement, false);
      });

      test('basic category tests', function() {
        for (var key in settings.ContentSettingsTypes) {
          var category = settings.ContentSettingsTypes[key];

          // A quick testing note on a few special categories...

          // The USB Devices category has no global toggle -- and therefore no
          // Category Desc (and has a special way of storing its data).

          // The Protocol Handlers is a special category in that is does not
          // store its data like the rest, but it has a global default toggle
          // and therefore can be tested like the rest below.

          // Test category text ids and descriptions for those categories that
          // have those.
          if (category != settings.ContentSettingsTypes.USB_DEVICES &&
              category != settings.ContentSettingsTypes.ZOOM_LEVELS) {
            assertNotEquals('', testElement.computeCategoryDesc(
                category, settings.PermissionValues.ALLOW, true));
            assertNotEquals('', testElement.computeCategoryDesc(
                category, settings.PermissionValues.ALLOW, false));
            assertNotEquals('', testElement.computeCategoryDesc(
                category, settings.PermissionValues.BLOCK, true));
            assertNotEquals('', testElement.computeCategoryDesc(
                category, settings.PermissionValues.BLOCK, false));

            // Test additional tri-state values:
            if (category == settings.ContentSettingsTypes.PLUGINS) {
              assertNotEquals('', testElement.computeCategoryDesc(
                  category, settings.PermissionValues.IMPORTANT_CONTENT, true));
              assertNotEquals('', testElement.computeCategoryDesc(
                  category, settings.PermissionValues.IMPORTANT_CONTENT,
                  false));
            } else if (category == settings.ContentSettingsTypes.COOKIES) {
              assertNotEquals('', testElement.computeCategoryDesc(
                  category, settings.PermissionValues.SESSION_ONLY, true));
              assertNotEquals('', testElement.computeCategoryDesc(
                  category, settings.PermissionValues.SESSION_ONLY, false));
            }
          }
        }
      });

      function testTristateCategory(prefs, category, thirdState,
                                    secondaryToggleId) {
        browserProxy.setPrefs(prefs);

        testElement.category = category;
        var secondaryToggle = null;

        return browserProxy.whenCalled('getDefaultValueForContentType').then(
          function(contentType) {
            Polymer.dom.flush();

            secondaryToggle = testElement.$$(secondaryToggleId);
            assertTrue(!!secondaryToggle);

            assertEquals(category, contentType);
            assertTrue(testElement.categoryEnabled);
            assertFalse(secondaryToggle.disabled);
            assertTrue(secondaryToggle.checked);

            MockInteractions.tap(testElement.$.toggle);
            return browserProxy.whenCalled('setDefaultValueForContentType');
          }).then(function(args) {
            // Check THIRD_STATE => BLOCK transition succeeded.
            Polymer.dom.flush();

            assertEquals(category, args[0]);
            assertEquals(settings.PermissionValues.BLOCK, args[1]);
            assertFalse(testElement.categoryEnabled);
            assertTrue(secondaryToggle.disabled);
            assertTrue(secondaryToggle.checked);

            browserProxy.resetResolver('setDefaultValueForContentType');
            MockInteractions.tap(testElement.$.toggle);
            return browserProxy.whenCalled('setDefaultValueForContentType');
          }).then(function(args) {
            // Check BLOCK => THIRD_STATE transition succeeded.
            Polymer.dom.flush();

            assertEquals(category, args[0]);
            assertEquals(thirdState, args[1]);
            assertTrue(testElement.categoryEnabled);
            assertFalse(secondaryToggle.disabled);
            assertTrue(secondaryToggle.checked);

            browserProxy.resetResolver('setDefaultValueForContentType');
            MockInteractions.tap(secondaryToggle);
            return browserProxy.whenCalled('setDefaultValueForContentType');
          }).then(function(args) {
            // Check THIRD_STATE => ALLOW transition succeeded.
            Polymer.dom.flush();

            assertEquals(category, args[0]);
            assertEquals(
                settings.PermissionValues.ALLOW, args[1]);
            assertTrue(testElement.categoryEnabled);
            assertFalse(secondaryToggle.disabled);
            assertFalse(secondaryToggle.checked);

            browserProxy.resetResolver('setDefaultValueForContentType');
            MockInteractions.tap(testElement.$.toggle);
            return browserProxy.whenCalled('setDefaultValueForContentType');
          }).then(function(args) {
            // Check ALLOW => BLOCK transition succeeded.
            Polymer.dom.flush();

            assertEquals(category, args[0]);
            assertEquals(settings.PermissionValues.BLOCK, args[1]);
            assertFalse(testElement.categoryEnabled);
            assertTrue(secondaryToggle.disabled);
            assertFalse(secondaryToggle.checked);

            browserProxy.resetResolver('setDefaultValueForContentType');
            MockInteractions.tap(testElement.$.toggle);
            return browserProxy.whenCalled('setDefaultValueForContentType');
          }).then(function(args) {
            // Check BLOCK => ALLOW transition succeeded.
            Polymer.dom.flush();

            assertEquals(category, args[0]);
            assertEquals(settings.PermissionValues.ALLOW, args[1]);
            assertTrue(testElement.categoryEnabled);
            assertFalse(secondaryToggle.disabled);
            assertFalse(secondaryToggle.checked);

            browserProxy.resetResolver('setDefaultValueForContentType');
            MockInteractions.tap(secondaryToggle);
            return browserProxy.whenCalled('setDefaultValueForContentType');
          }).then(function(args) {
            // Check ALLOW => THIRD_STATE transition succeeded.
            Polymer.dom.flush();

            assertEquals(category, args[0]);
            assertEquals(thirdState, args[1]);
            assertTrue(testElement.categoryEnabled);
            assertFalse(secondaryToggle.disabled);
            assertTrue(secondaryToggle.checked);
          });
      }

      test('test special tri-state Flash category', function() {
        return testTristateCategory(
            prefsFlashDetect, settings.ContentSettingsTypes.PLUGINS,
            settings.PermissionValues.IMPORTANT_CONTENT, '#flashAskToggle');
      });

      test('test special tri-state Cookies category', function() {
        return testTristateCategory(
            prefsCookesSessionOnly, settings.ContentSettingsTypes.COOKIES,
            settings.PermissionValues.SESSION_ONLY, '#sessionOnlyToggle');
      });
    });
  }

  return {
    registerTests: registerTests,
  };
});
