/* -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
 *  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 *
 *  Copyright (C) 2019 Red Hat, Inc.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <pthread.h>
#include <cpprest/http_client.h>

using namespace std;

class Arguments {
  istringstream stream;
public:
  vector<string> parts;

  Arguments(const string &in)
    : stream { in }, parts { }
  {
    string part;
    while (getline(stream, part, '.')) {
      parts.push_back(part);
    }
  }
};

int main(int, char**)
{
  web::http::client::http_client client("https://zuul.opendev.org");

  string hostname;
  while (getline(cin, hostname)) {
    // Expected hostname:
    // site.75031cad206c4014ad7a3387091d15ab.openstack.preview.opendev.org
    // Apache will drop "preview.opendev.org", so our expected input will be:
    // site.75031cad206c4014ad7a3387091d15ab.openstack

    Arguments arguments(hostname);
    if (arguments.parts.size() < 3) {
      cout << "not enough args" << endl;
      continue;
    }
    auto artifact = arguments.parts[0];
    auto buildid = arguments.parts[1];
    auto tenant = arguments.parts[2];
    cout << artifact << endl
         << buildid << endl
         << tenant << endl;

     // 75031cad206c4014ad7a3387091d15ab
    auto uri = web::uri_builder("/api/tenant/" + tenant + "/build");
    uri.append_path(buildid);
    auto response = client.request(
        web::http::methods::GET, uri.to_string()).get();
    // body is a web::json::value
    auto body = response.extract_json().get();
    cout << response.status_code() << endl;
    cout << body.serialize() << endl;

    // TODO: use artifact
    // body["log_url"].as_string() returns a const std::string&
    cout << body["log_url"].as_string() << endl;
  }
}
