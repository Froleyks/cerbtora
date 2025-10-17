#include "btor2.hpp"

#include <cassert>
#include <charconv>
#include <cstring>
#include <iostream>

Btor2::Btor2(const char *path) {
  parser = btor2parser_new();
  FILE *circuit_file = fopen(path, "r");
  if (!circuit_file) {
    std::cerr << "Cerbtora: Error: failed to open circuit." << std::endl;
    btor2parser_delete(parser);
    exit(1);
  }
  if (!btor2parser_read_lines(parser, circuit_file)) {

    std::cerr << "Cerbtora: Error: failed to parse circuit." << std::endl;
    fclose(circuit_file);
    btor2parser_delete(parser);
    exit(1);
  }
  fclose(circuit_file);
  for (auto &l : *this) {
    num_states += l.tag == BTOR2_TAG_state;
    num_inputs += l.tag == BTOR2_TAG_input;
  }
  max_id = btor2parser_max_id(parser);
}

Btor2::~Btor2() { btor2parser_delete(parser); }

std::ostream &operator<<(std::ostream &os, const Btor2Line &line) {
  os << line.id << " " << line.name;
  if (line.tag == BTOR2_TAG_sort) {
    os << " " << line.sort.name;
    if (line.sort.tag == BTOR2_TAG_SORT_bitvec)
      os << " " << line.sort.bitvec.width;
    else
      os << " " << line.sort.array.index << " " << line.sort.array.element;
  } else if (line.sort.id)
    os << " " << line.sort.id;
  // args[1] = ext but nargs only set to 1
  uint32_t nargs = line.nargs;
  if (line.tag == BTOR2_TAG_uext || line.tag == BTOR2_TAG_sext) nargs += 1;
  if (line.tag == BTOR2_TAG_slice) nargs += 2;
  for (uint32_t i = 0; i < nargs; ++i)
    os << " " << line.args[i];
  if (line.constant) os << " " << line.constant;
  if (line.symbol) os << " " << line.symbol;
  os << '\n';
  return os;
}

std::ostream &operator<<(std::ostream &os, const Btor2 &circuit) {
  for (const auto &l : circuit)
    os << l;
  return os;
}

int64_t
Btor2::reindex(int64_t offset,
               const std::vector<std::pair<int64_t, int64_t>> &pre_indexed) {
  std::vector<int64_t> map(size());
  for (auto [from, to] : pre_indexed)
    map.at(from) = to;
  auto m = [&map](int64_t &id) {
    if (!id) return;
    int64_t sign = id < 0 ? -1 : 1;
    assert(map.at(std::abs(id)));
    id = map.at(std::abs(id));
    id *= sign;
  };
  for (auto &l : *this) {
    if (l.tag == BTOR2_TAG_init || l.tag == BTOR2_TAG_next ||
        l.tag == BTOR2_TAG_bad || l.tag == BTOR2_TAG_output ||
        l.tag == BTOR2_TAG_constraint || l.tag == BTOR2_TAG_fair ||
        l.tag == BTOR2_TAG_justice) {
      if (l.tag == BTOR2_TAG_bad) bads.push_back(map.at(l.args[0]));
      if (l.tag == BTOR2_TAG_constraint)
        constraints.push_back(map.at(l.args[0]));
      drop(l.id);
      continue;
    }
    if (!map.at(l.id)) map.at(l.id) = ++offset;
    m(l.id);
    m(l.sort.id);
    if (l.sort.tag == BTOR2_TAG_SORT_array) {
      m(l.sort.array.index);
      m(l.sort.array.element);
    }
    for (int32_t i = 0; i < l.nargs; ++i)
      m(l.args[i]);
  }
  for (auto &l : *this) {
    if (l.tag != BTOR2_TAG_state) continue;
    m(l.init);
    m(l.next);
  }
  max_id = offset;
  return offset;
}

std::vector<std::pair<Btor2Line, int64_t>> Btor2::get_simulation() {
  std::vector<std::pair<Btor2Line, int64_t>> simulation;
  simulation.reserve(num_inputs + num_states);
  for (const auto &l : *this) {
    // This may be lifted
    if (l.tag != BTOR2_TAG_input && l.tag != BTOR2_TAG_state) continue;
    const char *name = l.symbol;
    if (!name) continue;
    const size_t len = strlen(name);
    if (len < 2 || name[0] != '=') continue;
    int64_t simulated_id{};
    [[maybe_unused]] auto [_, err] =
        std::from_chars(name + 1, name + len, simulated_id);
    assert(err == std::errc());
    simulation.emplace_back(l, simulated_id);
  }
  return simulation;
}

std::vector<std::pair<Btor2Line, int64_t>>
Btor2::get_default_simulation(const Btor2 &simulated) {
  std::vector<std::pair<Btor2Line, int64_t>> simulation;
  auto simulated_inputs = std::views::zip(*this | inputs, simulated | inputs);
  for (const auto &[w, m] :
       std::views::zip(*this | inputs, simulated | inputs)) {
    simulation.emplace_back(w, m.id);
  }
  for (const auto &[w, m] :
       std::views::zip(*this | states, simulated | states)) {
    simulation.emplace_back(w, m.id);
  }
  return simulation;
}
