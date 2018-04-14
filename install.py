# installer for wxobs
# Copyright 2016 Matthew Wall
# Co-opted by Glenn McKechnie 2017
# Distributed under the terms of the GNU Public License (GPLv3)

from setup import ExtensionInstaller

def loader():
    return wxobsInstaller()

class wxobsInstaller(ExtensionInstaller):
    def __init__(self):
        super(wxobsInstaller, self).__init__(
            version="0.7.0",
            name='wxobs',
            description='Generates a php report to show daily summaries',
            author="Glenn McKechnie",
            author_email="glenn.mckechnie@gmail.com",
            config={
                'StdReport': {
                    'wxobs': {
                        'skin': 'wxobs',
                        'HTML_ROOT': 'wxobs'
                        }}},
            files=[('bin/user',
                   ['bin/user/wxobs.py']),
                   ('skins/wxobs',
                   ['skins/wxobs/skin.conf',
                    'skins/wxobs/datepicker.css',
                    'skins/wxobs/datepicker.js',
                    'skins/wxobs/index.php.tmpl',
                    'skins/wxobs/links.inc',
                    'skins/wxobs/wxobs.css',
                    'skins/wxobs/wxobs.inc',
                    ]),
                   ('skins/wxobs/font',
                   ['skins/wxobs/font/OpenSans.woff',
                    'skins/wxobs/font/OpenSans.woff2',
                    ])
                   ]
        )
