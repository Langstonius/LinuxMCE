package Slim::Menu::SystemInfo;

# $Id: $

# Logitech Media Server Copyright 2001-2011 Logitech.
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License,
# version 2.

# Provides OPML-based extensible menu for system information

=head1 NAME

Slim::Menu::SystemInfo

=head1 DESCRIPTION

Provides a dynamic OPML-based system (server, players, controllers)
info menu to all UIs and allows plugins to register additional menu items.

=cut

use strict;
use Config;

use base qw(Slim::Menu::Base);

use Slim::Player::Client;
use Slim::Utils::Log;
use Slim::Utils::Prefs;
use Slim::Utils::Strings qw(cstring);
use Slim::Utils::Network;

my $log = logger('menu.systeminfo');
my $prefs = preferences('server');

# some SN values
my $_ss_version = 'r0';
my $_sn_version = 'r0';
my $_versions_mtime = 0;
my $_versions_last_checked = 0;

sub init {
	my $class = shift;
	$class->SUPER::init();
	
	Slim::Control::Request::addDispatch(
		[ 'systeminfo', 'items', '_index', '_quantity' ],
		[ 0, 1, 1, \&cliQuery ]
	);
}


sub name {
	return 'INFORMATION';
}

##
# Register all the information providers that we provide.
# This order is defined at http://wiki.slimdevices.com/index.php/UserInterfaceHierarchy
#
sub registerDefaultInfoProviders {
	my $class = shift;

	$class->SUPER::registerDefaultInfoProviders();
	
	if ( main::SLIM_SERVICE ) {
		$class->registerInfoProvider( squeezenetwork => (
			after => 'top',
			func  => \&infoSqueezeNetwork,
		) );
	
		$class->registerInfoProvider( currentplayer => (
			after => 'squeezenetwork',
			func  => \&infoCurrentPlayer,
		) );
	}
	
	else {		
		$class->registerInfoProvider( server => (
			after => 'top',
			func  => \&infoServer,
		) );
	
		$class->registerInfoProvider( library => (
			after => 'server',
			func  => \&infoLibrary,
		) );
		
		$class->registerInfoProvider( currentplayer => (
			after => 'library',
			func  => \&infoCurrentPlayer,
		) );
		
		$class->registerInfoProvider( players => (
			after => 'currentplayer',
			func  => \&infoPlayers,
		) );
		
		$class->registerInfoProvider( dirs => (
			after => 'players',
			func  => \&infoDirs,
		) );
		
		$class->registerInfoProvider( logs => (
			after => 'dirs',
			func  => \&infoLogs,
		) );
		
		$class->registerInfoProvider( plugins => (
			after => 'logs',
			func  => \&infoPlugins,
		) );
	}
	
}

sub infoPlayers {
	my $client = shift;
	my $tags   = shift;
	
	my @players = Slim::Player::Client::clients();
	return {} if ! scalar @players;
	
	my $item = {
		name  => cstring($client, 'INFORMATION_MENU_PLAYER'),
		items => []
	};
	
	for my $player (sort { $a->name cmp $b->name } @players) {
		
		my ($raw, $details) = _getPlayerInfo($player, $tags);
					
		push @{ $item->{items} }, {
			name  => $player->name,
			items => $details,
			web   => {
				name  => $player->name,
				items => $raw,
			} 
		}
	}
	
	return $item;
}

sub infoCurrentPlayer {
	my $client = shift || return;
	my $tags   = shift;
	
	my ($raw, $details) = _getPlayerInfo($client, $tags);
	
	my $item = {
		name  => cstring($client, 'INFORMATION_SPECIFIC_PLAYER', $client->name),
		items => $details,
		web   => {
			hide => 1,
			items => $raw,
		} 
	};
	
	return $item;
}

sub _getPlayerInfo {
	my $client = shift;
	my $tags   = shift;
	
	my $info = [
#		{ INFORMATION_PLAYER_NAME_ABBR       => $client->name },
		{ INFORMATION_PLAYER_MODEL           => $client->modelName },
		{ INFORMATION_FIRMWARE_ABBR          => $client->revision },
		{ INFORMATION_PLAYER_IP              => $client->ip },
#		{ INFORMATION_PLAYER_PORT            => $client->port },
		{ INFORMATION_PLAYER_MAC             => $client->macaddress },
		{ INFORMATION_PLAYER_SIGNAL_STRENGTH => $client->signalStrength ? $client->signalStrength . '%' : undef },
		{ INFORMATION_PLAYER_VOLTAGE         => $client->voltage },
	];

	my @details;
	foreach (@$info) {
		my ($key, $value) = each %{$_};
			
		next unless defined $value;
			
		if (Slim::Utils::Strings::stringExists($key . '_ABBR')) {
			$key = $key . '_ABBR'
		}
			
		push @details, {
			type => 'text',
			name => cstring($client, $key) . cstring($client, 'COLON') . ' ' . $value,
		};
	}

	if (Slim::Utils::PluginManager->isEnabled('Slim::Plugin::Health::Plugin')) {
		
		if (my $netinfo = Slim::Plugin::Health::NetTest::systemInfoMenu($client, $tags)) {
			push @details, $netinfo;
		}
	}
	
	return ($info, \@details);
}

sub infoLibrary {
	my $client = shift;
	
	if (!Slim::Schema::hasLibrary()) {
		return {
			name => cstring($client, 'INFORMATION_MENU_LIBRARY'),
			items => [
				{
					type => 'text',
					name => cstring($client, 'NO_LIBRARY'),
				},
			],
		};
	}
	
	elsif (Slim::Music::Import->stillScanning) {
		return {
			name => cstring($client, 'RESCANNING_SHORT'),
	
			web  => {
				hide => 1,
			},
		} 
	}
	
	my $totals = Slim::Schema->totals;
	
	my $items = {
		name => cstring($client, 'INFORMATION_MENU_LIBRARY'),

		items => [
			{
				type => 'text',
				name => cstring($client, 'INFORMATION_TRACKS') . cstring($client, 'COLON') . ' '
							. Slim::Utils::Misc::delimitThousands($totals->{track}),
			},

			{
				type => 'text',
				name => cstring($client, 'INFORMATION_ALBUMS') . cstring($client, 'COLON') . ' '
							. Slim::Utils::Misc::delimitThousands($totals->{album}),
			},

			{
				type => 'text',
				name => cstring($client, 'INFORMATION_ARTISTS') . cstring($client, 'COLON') . ' '
							. Slim::Utils::Misc::delimitThousands($totals->{'contributor'}),
			},

			{
				type => 'text',
				name => cstring($client, 'INFORMATION_GENRES') . cstring($client, 'COLON') . ' '
							. Slim::Utils::Misc::delimitThousands($totals->{genre}),
			},

			{
				type => 'text',
				name => cstring($client, 'INFORMATION_TIME') . cstring($client, 'COLON') . ' '
							. Slim::Utils::DateTime::timeFormat(Slim::Schema->totalTime),
			},
		],

		web  => {
			group  => 'library',
			unfold => 1,
		},
	};

	# don't bother counting images/videos unless media are enabled
	if ( main::MEDIASUPPORT ) {
		my ($request, $results);
		
		# XXX - no simple access to result sets for images/videos yet?
		if (main::VIDEO) {
			$request = Slim::Control::Request::executeRequest( $client, ['video_titles', 0, 0] );
			$results = $request->getResults();
		
			unshift @{ $items->{items} }, {
				type => 'text',
				name => cstring($client, 'INFORMATION_VIDEOS') . cstring($client, 'COLON') . ' '
					. ($results && $results->{count} ? Slim::Utils::Misc::delimitThousands($results->{count}) : 0),
			};
		}
	
		if (main::IMAGE) {
			$request = Slim::Control::Request::executeRequest( $client, ['image_titles', 0, 0] );
			$results = $request->getResults();
		
			unshift @{ $items->{items} }, {
				type => 'text',
				name => cstring($client, 'INFORMATION_IMAGES') . cstring($client, 'COLON') . ' '
					. ($results && $results->{count} ? Slim::Utils::Misc::delimitThousands($results->{count}) : 0),
			};
		}
	}
	
	return $items;
}

sub infoServer {
	my $client = shift;
	my $tags   = shift;
	
	my $menu   = $tags->{menuMode};
		
	my $osDetails = Slim::Utils::OSDetect::details();
	
	my $items = [
		{
			type => 'text',
			name => sprintf("Logitech Media Server %s%s %s - %s @ %s",
						cstring($client, 'INFORMATION_VERSION'),
						cstring($client, 'COLON'),
						$::VERSION,
						$::REVISION,
						$::BUILDDATE),
		},
		
		{
			type => 'text',
			name => cstring($client, 'INFORMATION_HOSTNAME') . cstring($client, 'COLON') . ' '
						. Slim::Utils::Network::hostName(),
		}, 
		
		{
			type => 'text',
			name => cstring($client, 'INFORMATION_SERVER_IP' . ($menu ? '_ABBR' : '')) . cstring($client, 'COLON') . ' '
						. Slim::Utils::Network::serverAddr(),
		}, 
		
		{
			type => 'text',
			name => cstring($client, 'INFORMATION_SERVER_HTTP' . ($menu ? '_ABBR' : '')) . cstring($client, 'COLON') . ' '
						. $prefs->get('httpport'),
		}, 
	
		{
			type => 'text',
			name => sprintf("%s%s %s - %s - %s ", 
						cstring($client, 'INFORMATION_OPERATINGSYSTEM' . ($menu ? '_ABBR' : '')),
						cstring($client, 'COLON'),
						$osDetails->{'osName'},
						$prefs->get('language'),
						Slim::Utils::Unicode::currentLocale()),
		},
		
		{
			type => 'text',
			name => cstring($client, 'INFORMATION_ARCHITECTURE' . ($menu ? '_ABBR' : '')) . cstring($client, 'COLON') . ' '
						. ($osDetails->{'osArch'} ? $osDetails->{'osArch'} : 'unknown'),
		},
		
		{
			type => 'text',
			name => cstring($client, 'PERL_VERSION') . cstring($client, 'COLON') . ' '
						. $Config{'version'} . ' - ' . $Config{'archname'},
		},
	];
	
	if ( Slim::Schema::hasLibrary() ) {
		push @{$items},	{
			type => 'text',
			name => cstring($client, 'DATABASE_VERSION') . cstring($client, 'COLON') . ' '
						. Slim::Utils::OSDetect->getOS->sqlHelperClass->sqlVersionLong( Slim::Schema->dbh ),
		};
	}
	
	push @{$items},	{
		type => 'text',
		name => cstring($client, 'INFORMATION_CLIENTS') . cstring($client, 'COLON') . ' '
					. Slim::Player::Client::clientCount,
	};

	return {
		name  => cstring($client, 'INFORMATION_MENU_SERVER'),
		items => $items,
	};
}

sub infoDirs {
	my $client = shift;
	
	my $folders = [
		{ INFORMATION_CACHEDIR   => $prefs->get('cachedir') },
		{ INFORMATION_PREFSDIR   => Slim::Utils::Prefs::dir() },
		# don't display SC's own plugin folder - the user shouldn't care about it
		{ INFORMATION_PLUGINDIRS => join(", ", grep {$_ !~ m|Slim/Plugin|} Slim::Utils::OSDetect::dirsFor('Plugins')) },
	];
	
	my $item = {
		name  => cstring($client, 'FOLDERS'),
		items => [],
		
		web   => {
			name  => 'FOLDERS',
			items => $folders,
		}
	};
	
	foreach (@$folders) {
		my ($key, $value) = each %{$_};
		push @{ $item->{items} }, {
			type => 'text',
			name => cstring($client, $key) . cstring($client, 'COLON') . ' ' . $value,
		}
	}
	
	return $item;
}

sub infoPlugins {
	my $client = shift || return;
	my $tags   = shift;
	
	my $item = {
		name  => cstring($client, 'INFORMATION_MENU_MODULE'),
		items => [],
		
		web   => {
			hide => 1,
		} 
	};

	my $plugins = Slim::Utils::PluginManager->allPlugins;
	my $pluginState = preferences('plugin.state')->all();

	my @list;

	for my $plugin (keys %$plugins) {

		# only show plugins with perl modules

		if ($plugins->{$plugin}->{'module'}) {

			my $name   = $plugins->{$plugin}->{'name'};
			my $version = $plugins->{$plugin}->{'version'};
			
			if ( $name eq uc($name) ) {
				$name = cstring($client, $name);
			}

			push @list, {
				type => 'text',
				name => $name . ' - v' . $version,
			}
		}
	}

	@list = sort { $a->{'name'} cmp $b->{'name'} } @list;

	$item->{'items'} = \@list;
	
	return $item;
}


sub infoLogs {
	my $client = shift;
	
	my $logs = Slim::Utils::Log->getLogFiles();
	
	my $item = {
		name  => cstring($client, 'SETUP_DEBUG_SERVER_LOG'),
		items => [],
		
		web   => {
			name  => 'SETUP_DEBUG_SERVER_LOG',
			items => $logs,
		}
	};
	
	foreach (@$logs) {
		my ($key, $value) = each %{$_};
		
		next unless $value;
		
		push @{ $item->{items} }, {
			type => 'text',
			name => cstring($client, "SETUP_DEBUG_${key}_LOG") . cstring($client, 'COLON') . ' ' . $value
		}
	}
	
	return $item;
}

sub infoSqueezeNetwork {
	my $client = shift;
	my $item;
	
	if ( main::SLIM_SERVICE ) {

		my $time = time();

		if ( ($time - $_versions_last_checked) > 60 ) {
			$_versions_last_checked = $time;

			my @stats = stat('/etc/sn/versions');
			my $mtime = $stats[9] || -1;

			if ( $mtime != $_versions_mtime ) {
				$_versions_mtime = $mtime;

				my $ok = open(my $vfile, '<', '/etc/sn/versions');
				
				if ($ok) {
					local $_;
					while(<$vfile>) {
						chomp;
						next unless /^(S[NS]):([^:]+)$/;
						$_sn_version = $2 if $1 eq 'SN';

						# SS version is only read once because this instance may
						# be running an older version than the server has
						if ( $_ss_version eq 'r0' ) {
							$_ss_version = $2 if $1 eq 'SS';
						}
					}
					
					close($vfile);
				}
			}
		}

		my $config = SDI::Util::SNConfig::get_config();
		my $dcname = $config->{dcname};
		my $dcdesc = $config->{datacenter_info}->{$dcname}->{desc} || 'Unknown';

		$item = {
			name  => cstring($client, 'INFORMATION_MENU_SERVER'),
			items => [
				{
					type => 'text',
					name => sprintf('%s SS%s %s, SN%s %s',
						cstring($client, 'SERVER_VERSION'),
						cstring($client, 'COLON'),
						$_ss_version,
						cstring($client, 'COLON'),
						$_sn_version					
					)
				},
				
				{
					type => 'text',
					name => cstring($client, 'INFORMATION_DATACENTER') . cstring($client, 'COLON') . ' ' . $dcdesc,
				},
				
				{
					type => 'text',
					# don't show the anonymous account information
					name => cstring($client, 'INFORMATION_SN_ACCOUNT') . cstring($client, 'COLON') . ' ' . 
						( $client->playerData->userid->email =~ /^!anonymous_[\da-f]+$/i 
							? cstring($client, 'NONE') 
							: $client->playerData->userid->email
						),
				}
				
			]
		};
		
	}
	
	return $item;
}

sub cliQuery {
	my $request = shift;
	
	my $client  = $request->client;
	my $tags    = {
		menuMode => $request->getParam('menu') || 0,
	};
	my $feed    = Slim::Menu::SystemInfo->menu( $client, $tags );

	Slim::Control::XMLBrowser::cliQuery('systeminfo', $feed, $request );
}

1;
