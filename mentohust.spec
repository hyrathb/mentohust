Name:           mentohust
Version:        0.2.4
Release:        1%{?dist}
Summary:        锐捷和塞尔认证

Group:          Applications/Internet
License:        GPL
URL:            http://code.google.com/p/mentohust/
Source0:        mentohust-%{version}.tar.bz2
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  libpcap-devel glibc-devel 
Requires:       libpcap glibc 

%description	
				mentohust 是用来进行 锐捷和塞尔认证的。因为官方没有Linux版本或者Linux 版本很不好用 ^_^


%prep
%setup -q


%build
%configure
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
make install_conf DESTDIR=$RPM_BUILD_ROOT
chmod +s $RPM_BUILD_ROOT/usr/bin/mentohust

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_prefix}/bin/mentohust
/etc/mentohust.conf
%{_prefix}/share/doc/mentohust/README
%doc

%changelog
