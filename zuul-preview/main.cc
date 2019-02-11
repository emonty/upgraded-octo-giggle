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

#include <boost/optional.hpp>
#include <cpprest/http_client.h>
#include <bits/stdc++.h>

using namespace std;

vector<string> split(const string &in)
{
  istringstream stream{in};
  vector<string> parts{};
  string part{};
  while (getline(stream, part, '.')) {
    parts.push_back(part);
  }
  return parts;
}

class Cache {
  // A queue of hostname, URL pairs.  The head of the queue is always
  // the most recently accessed entry, the tail is the least.
  list<pair<const string, const string>> queue;

  // A map of hostname -> iterator that points into the queue, for
  // quick lookup.
  unordered_map<string, list<pair<const string, const string>>::iterator> map;

  // The maximum size of the cache.
  const uint32_t size;

public:
  Cache(uint s)
    : queue {}, map {}, size{s}
  { }

  // Lookup the hostname in the cache and return the URL if present.
  // If the entry is present, it is moved to the head of the queue.
  boost::optional<const string> get(const string &key)
  {
    auto location{map.find(key)};
    if (location == map.end())
      return {};

    auto val{*(location->second)};
    queue.erase(location->second);
    queue.push_front(val);
    //cout << "get push " << val.second << endl;
    return val.second;
  }

  // Add an entry to the cache.  If the cache is full, drop the least
  // recently used entry.
  void put(const string &key, const string &value)
  {
    auto location{map.find(key)};
    if (location != map.end())
      return;

    if (queue.size() == size) {
      auto last{queue.back()};
      //cout << "put pop " << last.second << endl;
      queue.pop_back();
      map.erase(last.first);
    }

    //cout << "put push " << value << endl;
    queue.push_front(make_pair(key, value));
    map[key] = queue.begin();
  }
};


int main(int, char**)
{
  web::http::client::http_client client{"https://zuul.opendev.org"};

  string hostname;
  Cache cache{2};
  while (getline(cin, hostname)) {
    // Expected hostname:
    // site.75031cad206c4014ad7a3387091d15ab.openstack.preview.opendev.org
    // Apache will drop "preview.opendev.org", so our expected input will be:
    // site.75031cad206c4014ad7a3387091d15ab.openstack
    // site.7c16d914db5a4c4b91cd9a31d119dd48.openstack
    // site.688b70499b9a41a08f498ed6e932960c.openstack
    // site.dbefc23dcc594577a8bfa4db4f9b0a8f.openstack

    auto val{cache.get(hostname)};
    if (val) {
      cout << val.value() << endl;
      continue;
    }

    auto parts{split(hostname)};
    if (parts.size() < 3) {
      cout << "not enough args" << endl;
      continue;
    }
    auto artifact{parts[0]};
    auto buildid{parts[1]};
    auto tenant{parts[2]};
    /*
    cout << artifact << endl
         << buildid << endl
         << tenant << endl;
    */

     // 75031cad206c4014ad7a3387091d15ab
    try {
      web::uri_builder uri{"/api/tenant/" + tenant + "/build"};
      uri.append_path(buildid);
      auto response{client.request(
        web::http::methods::GET, uri.to_string()).get()};
      // body is a web::json::value
      // cout << response.status_code() << endl;
      auto body{response.extract_json().get()};
      //cout << body.serialize() << endl;

      // TODO: use artifact
      // body["log_url"].as_string() returns a const std::string&
      cout << body["log_url"].as_string() << endl;

      cache.put(hostname, body["log_url"].as_string());
    } catch (...) {
      cout << "error" << endl;
    }
  }
}
