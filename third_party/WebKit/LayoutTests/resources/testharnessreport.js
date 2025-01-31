/*
 * THIS FILE INTENTIONALLY LEFT BLANK
 *
 * More specifically, this file is intended for vendors to implement
 * code needed to integrate testharness.js tests with their own test systems.
 *
 * Typically such integration will attach callbacks when each test is
 * has run, using add_result_callback(callback(test)), or when the whole test file has
 * completed, using add_completion_callback(callback(tests, harness_status)).
 *
 * For more documentation about the callback functions and the
 * parameters they are called with see testharness.js
 */

(function() {

    var output_document = document;

    // Setup for WebKit JavaScript tests
    if (self.testRunner) {
        testRunner.dumpAsText();
        testRunner.waitUntilDone();
        testRunner.setCanOpenWindows();
        testRunner.setCloseRemainingWindowsWhenComplete(true);
        testRunner.setDumpJavaScriptDialogs(false);
    }

    // Disable the default output of testharness.js.  The default output formats
    // test results into an HTML table.  When that table is dumped as text, no
    // spacing between cells is preserved, and it is therefore not readable. By
    // setting output to false, the HTML table will not be created.
    // Also, disable timeout (except for explicit timeout), since the Blink
    // layout test runner has its own timeout mechanism.
    // See: https://github.com/w3c/testharness.js/blob/master/docs/api.md#setup
    setup({
        "output": false,
        "explicit_timeout": true
    });

    // Function used to convert the test status code into the corresponding
    // string
    function convertResult(resultStatus) {
        switch (resultStatus) {
        case 0:
            return "PASS";
        case 1:
            return "FAIL";
        case 2:
            return "TIMEOUT";
        default:
            return "NOTRUN";
        }
    }

    var localPathRegExp;
    if (document.URL.startsWith("file:///")) {
        var index = document.URL.indexOf("/imported/wpt");
        if (index >= 0) {
            var localPath = document.URL.substring("file:///".length, index + "/imported/wpt".length);
            localPathRegExp = new RegExp(localPath.replace(/(\W)/g, "\\$1"), "g");
        }
    }

    // Sanitizes the given text for display in test results.
    function sanitize(text) {
        if (!text) {
            return "";
        }
        // Escape null characters, otherwise diff will think the file is binary.
        text = text.replace(/\0/g, "\\0");
        // Escape carriage returns as they break rietveld's difftools.
        text = text.replace(/\r/g, "\\r");
        // Replace machine-dependent path with "...".
        if (localPathRegExp)
            text = text.replace(localPathRegExp, "...");
        return text;
    }

    // If the test has a meta tag named flags and the content contains "dom",
    // then it's a CSSWG test.
    function isCSSWGTest() {
        var flags = document.querySelector('meta[name=flags]'),
            content = flags ? flags.getAttribute('content') : null;
        return content && content.match(/\bdom\b/);
    }

    function isJSTest() {
        return !!document.querySelector('script[src*="/resources/testharness"]');
    }

    function isWPTManualTest() {
        var path = location.pathname;
        if (location.hostname == 'web-platform.test' && path.endsWith('-manual.html'))
            return true;
        return /\/imported\/wpt\/.*-manual\.html$/.test(path);
    }

    // Returns a directory part relative to WPT root and a basename part of the
    // current test. e.g.
    // Current test: file:///.../LayoutTests/imported/wpt/pointerevents/foobar.html
    // Output: "/pointerevents/foobar"
    function pathAndBaseNameInWPT() {
        var path = location.pathname;
        if (location.hostname == 'web-platform.test') {
            var matches = path.match(/^(\/.*)\.html$/);
            return matches ? matches[1] : null;
        }
        var matches = path.match(/imported\/wpt(\/.*)\.html$/);
        return matches ? matches[1] : null;
    }

    function loadAutomationScript() {
        var pathAndBase = pathAndBaseNameInWPT();
        if (!pathAndBase)
            return;
        var automationPath = location.pathname.replace(/\/imported\/wpt\/.*$/, '/imported/wpt_automation');
        if (location.hostname == 'web-platform.test')
            automationPath = '/wpt_automation';

        // Export importAutomationScript for use by the automation scripts.
        window.importAutomationScript = function(relativePath) {
            var script = document.createElement('script');
            script.src = automationPath + relativePath;
            document.head.appendChild(script);
        }

        var src;
        if (pathAndBase.startsWith('/fullscreen/')) {
            // Fullscreen tests all use the same automation script.
            src = automationPath + '/fullscreen/auto-click.js';
        } else if (pathAndBase.startsWith('/pointerevents/')
                   || pathAndBase.startsWith('/uievents/')) {
            // Per-test automation scripts.
            src = automationPath + pathAndBase + '-automation.js';
        } else {
            return;
        }
        var script = document.createElement('script');
        script.src = src;
        document.head.appendChild(script);
    }

    var didDispatchLoadEvent = false;
    window.addEventListener('load', function() {
        didDispatchLoadEvent = true;
        if (isWPTManualTest()) {
            setTimeout(loadAutomationScript, 0);
        }
    }, { once: true });

    add_start_callback(function(properties) {
      if (properties.output_document)
        output_document = properties.output_document;
    });

    // Using a callback function, test results will be added to the page in a
    // manner that allows dumpAsText to produce readable test results.
    add_completion_callback(function (tests, harness_status) {

        // Create element to hold results.
        var results = output_document.createElement("pre");

        // Declare result string.
        var resultStr = "This is a testharness.js-based test.\n";

        // Check harness_status.  If it is not 0, tests did not execute
        // correctly, output the error code and message.
        if (harness_status.status != 0) {
            resultStr += "Harness Error. harness_status.status = " +
                harness_status.status +
                " , harness_status.message = " +
                harness_status.message +
                "\n";
        }
        // reflection tests contain huge number of tests, and Chromium code
        // review tool has the 1MB diff size limit. We merge PASS lines.
        if (output_document.URL.indexOf("/html/dom/reflection") >= 0) {
            for (var i = 0; i < tests.length; ++i) {
                if (tests[i].status == 0) {
                    var colon = tests[i].name.indexOf(':');
                    if (colon > 0) {
                        var prefix = tests[i].name.substring(0, colon + 1);
                        var j = i + 1;
                        for (; j < tests.length; ++j) {
                            if (!tests[j].name.startsWith(prefix) || tests[j].status != 0)
                                break;
                        }
                        if ((j - i) > 1) {
                            resultStr += convertResult(tests[i].status) +
                                " " + sanitize(prefix) + " " + (j - i) + " tests\n"
                            i = j - 1;
                            continue;
                        }
                    }
                }
                resultStr += convertResult(tests[i].status) + " " +
                    sanitize(tests[i].name) + " " +
                    sanitize(tests[i].message) + "\n";
            }
        } else {
            // Iterate through tests array and build string that contains
            // results for all tests.
            for (var i = 0; i < tests.length; ++i) {
                resultStr += convertResult(tests[i].status) + " " +
                    sanitize(tests[i].name) + " " +
                    sanitize(tests[i].message) + "\n";
            }
        }

        resultStr += "Harness: the test ran to completion.\n";

        // Set results element's textContent to the results string.
        results.textContent = resultStr;

        function done() {
            if (self.testRunner) {
                // The following DOM operations may show console messages.  We
                // suppress them because they are not related to the running
                // test.
                testRunner.setDumpConsoleMessages(false);

                if (isCSSWGTest() || isJSTest()) {
                    // Anything isn't material to the testrunner output, so
                    // should be hidden from the text dump.
                    if (output_document.body && output_document.body.tagName == 'BODY')
                        output_document.body.textContent = '';
                }
            }

            // Add results element to output_document.
            if (!output_document.body || output_document.body.tagName != 'BODY') {
                if (!output_document.documentElement)
                    output_document.appendChild(output_document.createElement('html'));
                else if (output_document.body) // output_document.body is <frameset>.
                    output_document.body.remove();
                output_document.documentElement.appendChild(output_document.createElement("body"));
            }
            output_document.body.appendChild(results);

            if (self.testRunner)
                testRunner.notifyDone();
        }

        if (didDispatchLoadEvent || output_document.readyState != 'loading') {
            // This function might not be the last 'completion callback', and
            // another completion callback might generate more results.  So, we
            // don't dump the results immediately.
            setTimeout(done, 0);
        } else {
            // Parsing the test HTML isn't finished yet.
            window.addEventListener('load', done);
        }
    });

})();
