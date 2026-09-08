Name:       harbour-sailfish-link
Version:    0.1.2
Release:    1
Summary:    Sailfish-side local discovery and pairing service
License:    MIT
Group:      Applications/Connectivity
Source0:    %{name}-%{version}.tar.bz2

BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(sailfishapp)

Requires:   sailfishsilica-qt5

%description
Sailfish Link provides a small on-device control surface and a background
UDP announcement service for local mother-PC discovery.

%prep
%setup -q

%build
%qmake5
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
make install INSTALL_ROOT=%{buildroot}

%files
%defattr(-,root,root,-)
%{_bindir}/harbour-sailfish-link
%{_bindir}/harbour-sailfish-link-discovery
%{_datadir}/applications/harbour-sailfish-link.desktop
%{_datadir}/icons/hicolor/86x86/apps/harbour-sailfish-link.png
%{_datadir}/harbour-sailfish-link/qml
/usr/lib/systemd/user/harbour-sailfish-link.service

%changelog
* Tue Sep 08 2026 OfficeFake <dev@example.invalid> - 0.1.2-1
- Ship the launcher icon as a 86x86 PNG for Sailfish OS 3.2 app-grid support.

* Sun Sep 06 2026 OfficeFake <dev@example.invalid> - 0.1.1-1
- Route discovery broadcasts and multicast through the selected LAN interface.
- Preserve invalid configuration for diagnosis and restrict pairing-state permissions.

* Thu Jul 02 2026 OfficeFake <dev@example.invalid> - 0.1.0-1
- Initial Sailfish-side discovery scaffold.
