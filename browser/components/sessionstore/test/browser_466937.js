/* Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/ */

"use strict";

const URL = ROOT + "browser_466937_sample.html";

/**
 * Bug 466937 - Prevent file stealing with sessionstore.
 */
add_task(async function test_prevent_file_stealing() {
  // Add a tab with some file input fields.
  let tab = BrowserTestUtils.addTab(gBrowser, URL);
  let browser = tab.linkedBrowser;
  await promiseBrowserLoaded(browser);

  // Generate a path to a 'secret' file.
  let file = Services.dirsvc.get("TmpD", Ci.nsIFile);
  file.append("466937_test.file");
  file.createUnique(Ci.nsIFile.NORMAL_FILE_TYPE, 0o666);
  let testPath = file.path;

  // Fill in form values.
  await setInputValue(browser, {id: "reverse_thief", value: "/home/user/secret2"});
  await setInputValue(browser, {id: "bystander", value: testPath});

  // Duplicate and check form values.
  let tab2 = gBrowser.duplicateTab(tab);
  let browser2 = tab2.linkedBrowser;
  await promiseTabRestored(tab2);

  let thief = await getInputValue(browser2, {id: "thief"});
  is(thief, "", "file path wasn't set to text field value");
  let reverse_thief = await getInputValue(browser2, {id: "reverse_thief"});
  is(reverse_thief, "", "text field value wasn't set to full file path");
  let bystander = await getInputValue(browser2, {id: "bystander"});
  is(bystander, testPath, "normal case: file path was correctly preserved");

  // Cleanup.
  gBrowser.removeTab(tab);
  gBrowser.removeTab(tab2);
});
