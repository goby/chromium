# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from webkitpy.common.memoized import memoized
from webkitpy.common.webkit_finder import WebKitFinder


class ChromiumCommit(object):

    def __init__(self, host, sha=None, position=None):
        """
        Args:
            host: A Host object
            sha: A Chromium commit SHA
            position: A string of the form:
                    'Cr-Commit-Position: refs/heads/master@{#431915}'
                or just:
                    'refs/heads/master@{#431915}'
        """
        self.host = host

        assert sha or position, 'requires sha or position'
        assert not (sha and position), 'cannot accept both sha and position'

        if position and not sha:
            if position.startswith('Cr-Commit-Position: '):
                position = position[len('Cr-Commit-Position: '):]

            sha = self.position_to_sha(position)

        self.sha = sha
        self.position = position

    def num_behind_master(self):
        """Returns the number of commits this commit is behind origin/master.
        It is inclusive of this commit and of the latest commit.
        """
        return len(self.host.executive.run_command([
            'git', 'rev-list', '{}..origin/master'.format(self.sha)
        ]).splitlines())

    def position_to_sha(self, commit_position):
        return self.host.executive.run_command([
            'git', 'crrev-parse', commit_position
        ]).strip()

    def subject(self):
        return self.host.executive.run_command([
            'git', 'show', '--format=%s', '--no-patch', self.sha
        ])

    def body(self):
        return self.host.executive.run_command([
            'git', 'show', '--format=%b', '--no-patch', self.sha
        ])

    def message(self):
        """Returns a string with a commit's subject and body."""
        return self.host.executive.run_command([
            'git', 'show', '--format=%B', '--no-patch', self.sha
        ])

    def format_patch(self):
        """Makes a patch with just changes in files in the WPT for a given commit."""
        # TODO(jeffcarp): exclude expectations files
        # TODO(jeffcarp): exclude manifest files
        return self.host.executive.run_command([
            'git', 'format-patch', '-1', '--stdout',
            self.sha, self.absolute_chromium_wpt_dir()
        ])

    @memoized
    def absolute_chromium_wpt_dir(self):
        finder = WebKitFinder(self.host.filesystem)
        return finder.path_from_webkit_base('LayoutTests', 'imported', 'wpt')
