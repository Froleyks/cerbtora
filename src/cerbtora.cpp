#include "btor2.hpp"
#include "encoding.hpp"

#include <cstring>
#include <iostream>
#include <ranges>

#ifdef QUIET
#define MSG \
  if (0) std::cout
#else
#define MSG std::cout << "Cerbotor: "
#endif

std::ofstream Encoding::file;
int64_t Encoding::id, Encoding::bool_id, Encoding::true_id, Encoding::offset;

auto param(int argc, char *argv[]) {
  std::vector<const char *> checks{
      "reset.btor2", "transition.btor2", "property.btor2",
      "base.btor2",  "step.btor2",
  };
  if (argc > 1 && !strcmp(argv[1], "--version")) {
    std::cout << VERSION << '\n';
    exit(0);
  }
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <model.btor2> <witness.btor2> [";
    for (const char *o : checks)
      std::cerr << " <" << o << ">";
    std::cerr << " ]\n";
    exit(1);
  }
  for (int i = 3; i < argc; ++i)
    checks[i - 3] = argv[i];
  return std::tuple{argv[1], argv[2], checks};
}

std::vector<std::pair<Btor2Line, Btor2Line>> index_consecutively(Btor2 &witness,
                                                                 Btor2 &model) {
  int64_t offset = witness.reindex();
  auto simulation = witness.get_simulation();
  if (simulation.empty()) simulation = witness.get_default_simulation(model);
  std::vector<std::pair<int64_t, int64_t>> pre_indexded;
  pre_indexded.reserve(simulation.size());
  for (const auto &[l, m_id] : simulation)
    pre_indexded.emplace_back(m_id, l.id);
  model.reindex(offset, pre_indexded);
  std::vector<std::pair<Btor2Line, Btor2Line>> shared;
  shared.reserve(simulation.size());
  for (const auto &[l, m_id] : simulation) {
    shared.emplace_back(l, model.at(m_id));
    model.drop(m_id);
  }
  return shared;
}

void reset(const char *path, const Btor2 &witness, const Btor2 &model,
           const std::vector<std::pair<Btor2Line, Btor2Line>> &shared) {
  Encoding reset(path, witness, model);
  std::vector<int64_t> m_reset, w_reset;
  m_reset.reserve(shared.size());
  w_reset.reserve(shared.size());
  for (auto &[w, m] : shared) {
    if (w.init) w_reset.push_back(beq(w.id, w.init));
    if (m.init) m_reset.push_back(beq(m.id, m.init));
  }
  bbad(band(                             // R{K} ^ C ^ !(R'{K} ^ C')
      band(m_reset),                     // R{K}
      band(model.constraints),           // C
      bnot(band(                         //
          band(w_reset),                 // R'{K}
          band(witness.constraints))))); // C'
}

void transition(const char *path, const Btor2 &witness, const Btor2 &model,
                const std::vector<std::pair<Btor2Line, Btor2Line>> &shared) {
  Encoding transition(path, witness, model);
  unroll(witness);
  unroll(model);
  std::vector<int64_t> m_transition, w_transition;
  m_transition.reserve(shared.size());
  w_transition.reserve(shared.size());
  for (auto &[w, m] : shared) {
    if (w.next) w_transition.push_back(beq(next(w.id), w.next));
    if (m.next) m_transition.push_back(beq(next(m.id), m.next));
  }
  bbad(band(                         // F{K} ^ C0 ^ C1 ^ C0' ^ !(F'{K} ^ C1')
      band(m_transition),            // F{K}
      band(model.constraints),       // C0
      band(next(model.constraints)), // C1
      band(witness.constraints),     // C0'
      bnot(band(                     //
          band(w_transition),        // F'{K}
          band(next(witness.constraints)))))); // C1'
}

void property(const char *path, const Btor2 &witness, const Btor2 &model,
              const std::vector<std::pair<Btor2Line, Btor2Line>> &shared) {
  Encoding property(path, witness, model);
  bbad(band(                     // !P ^ C ^ C' ^ P'
      bor(model.bads),           // !P
      band(model.constraints),   // C
      band(witness.constraints), // C'
      bnot(bor(witness.bads)))); // P'
}

void base(const char *path, const Btor2 &witness) {
  Encoding base(path, witness);
  std::vector<int64_t> reset;
  reset.reserve(witness.num_states);
  for (const auto &[s, i] : witness | inits)
    reset.push_back(beq(s, i));
  bbad(band(                     // R' ^ C' ^ !P'
      band(reset),               // R'
      band(witness.constraints), // C'
      bor(witness.bads)));       // !P'
}

void step(const char *path, const Btor2 &witness) {
  Encoding step(path, witness);
  unroll(witness);
  std::vector<int64_t> transition;
  transition.reserve(witness.num_states);
  for (const auto &[s, n] : witness | nexts)
    transition.push_back(beq(next(s), n));
  bbad(band(                           // P0' ^ F' ^ C0' ^ C1' ^ !P1'
      bnot(bor(witness.bads)),         // P0'
      band(transition),                // F'
      band(witness.constraints),       // C0'
      band(next(witness.constraints)), // C1'
      bor(next(witness.bads))));       // !P1'
}

int main(int argc, char **argv) {
  auto [model_path, witness_path, checks] = param(argc, argv);
  MSG << "Certify Model Checking Witnesses in Btor2\n";
  MSG << VERSION " " GITID "\n";
  Btor2 model(model_path), witnesses(witness_path);
  const auto shared = index_consecutively(witnesses, model);
  reset(checks[0], witnesses, model, shared);
  transition(checks[1], witnesses, model, shared);
  property(checks[2], witnesses, model, shared);
  base(checks[3], witnesses);
  step(checks[4], witnesses);
}
