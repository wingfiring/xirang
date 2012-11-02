#include "typeparser.h"

#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_escape_char.hpp>
#include <boost/spirit/include/classic_multi_pass.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>
#include <boost/spirit/include/classic_chset.hpp>

#include <functional>
#include <iterator>
#include <boost/tokenizer.hpp>
#include <aio/xirang/typebinder.h>

#include <memory>
#include <stack>

AIO_EXCEPTION_TYPE(compiler_error);

namespace bsc = boost::spirit::classic;
using namespace xirang;

template<typename Iter>
class TypeLoaderActions
{
	public:
		TypeLoaderActions(Xirang& xr) : m_xr(&xr)
		{
			begin_type 	= [this](Iter first, Iter last) { this->begin_type_(first,last);};
			new_ns  	= [this](Iter first, Iter last) { this->new_ns_(first,last);};
			end_ns  	= [this](Iter first, Iter last) { this->end_ns_(first,last);};
			new_model  	= [this](Iter first, Iter last) { this->new_model_(first,last);};
			end_args 	= [this](Iter first, Iter last) { this->end_args_(first,last);};
			new_arg_name  = [this](Iter first, Iter last) { this->new_arg_name_(first,last);};
			new_arg_type  = [this](Iter first, Iter last) { this->new_arg_type_(first,last);};
			new_base  	= [this](Iter first, Iter last) { this->new_base_(first,last);};
			new_member  = [this](Iter first, Iter last) { this->new_member_(first,last);};
			new_member_type  = [this](Iter first, Iter last) { this->new_member_type_(first,last);};
			end_type  	= [this](Iter first, Iter last) { this->end_type_(first,last);};
			end_file  	= [this](Iter first, Iter last) { this->end_file_(first,last);};

			throw_not_type  	= [this](Iter first, Iter last) { this->throw_not_type_(first,last);};
			throw_not_model  	= [this](Iter first, Iter last) { this->throw_not_model_(first,last);};
			throw_not_arg  	= [this](Iter first, Iter last) { this->throw_not_arg_(first,last);};
			throw_not_base  	= [this](Iter first, Iter last) { this->throw_not_base_(first,last);};
			throw_not_member  	= [this](Iter first, Iter last) { this->throw_not_member_(first,last);};
			throw_not_ns  	= [this](Iter first, Iter last) { this->throw_not_ns_(first,last);};
			throw_not_file  	= [this](Iter first, Iter last) { this->throw_not_file_(first,last);};

            ns_current.push(xr.root());
		}

		void throw_(const std::string& msg, const aio::string& text)
		{
			throw_(msg.c_str(), text.c_str());
		}
		void throw_(const std::string& msg, const std::string& text)
		{
			throw_(msg.c_str(), text.c_str());
		}

		void throw_(const char* msg, const char* text)
		{
			AIO_THROW(compiler_error)(msg)(":")(text);
		}

		typedef std::function<void(Iter, Iter)> str_action;

		void begin_type_(Iter first, Iter last) {            
            tp_builder.name(aio::make_range(first,last));
		}

		void new_ns_(Iter first, Iter last) {
			std::string name(first, last);

            Namespace ns = ns_current.top();

			typedef boost::char_separator<char> separator;

			boost::tokenizer<separator> tok(name, separator("."));
			bool find = true;
			for (auto pos = tok.begin(); pos != tok.end(); ++pos)
			{
				if (find)
				{
					Namespace newns = ns.findNamespace(pos->c_str());
					if (newns.valid())
						ns = newns;
					else
						find = false;
				}
				if (!find) //create
				{
                    std::shared_ptr<NamespaceBuilder> pnb (new NamespaceBuilder);
                    pnb->name(pos->c_str());

                    ns_builders.push(std::make_pair(pnb, ns));
					ns = pnb->get();
				}
				if (!ns.valid())
					throw_("locate or create namespace failed", *pos);
			}

            ns_current.push(ns);
		}

		void end_ns_(Iter first, Iter last) {
            ns_builders.top().first->adoptBy(ns_builders.top().second);
            ns_builders.pop();
            ns_current.pop();
		}

		void new_model_(Iter first, Iter last) {
            tp_builder.modelFrom(ns_current.top().locateType(aio::make_range(first, last), '.'));
		}

		void end_args_(Iter first, Iter last) {
		}

		void new_arg_name_(Iter first, Iter last) {
			m_name = aio::make_range(first, last);
            
			if (tp_builder.get().arg(m_name).valid())
				throw_("failed to create type arg", m_name);
		}

		void new_arg_type_(Iter first, Iter last) {
			aio::string typeName = aio::make_range(first, last);
            tp_builder.setArg(m_name, typeName, findType_(typeName));
		}

		void new_base_(Iter first, Iter last) {
			aio::string name = aio::make_range(first, last);
            tp_builder.addBase(name, findType(name));
		}

		Type findType(const aio::string& name)
		{
			Type t = findType_(name);
			if (!t.valid() || !t.isComplete())
				throw_("type not found or not complete", name);

			return t;

		}
		Type findType_(const aio::string& name)
		{
            auto arg = tp_builder.get().arg(name);
			if (arg.valid())
			{
				return arg.type();
			}
            return ns_current.top().locateType(name, '.');
		}

		void new_member_(Iter first, Iter last) {
			m_name =aio::make_range(first, last);
		}

		void new_member_type_(Iter first, Iter last) {            
			aio::string typeName = aio::make_range(first, last);

            tp_builder.addMember(m_name, typeName, findType(typeName));
		}

		void end_type_(Iter first, Iter last) {
            tp_builder.endBuild();
            tp_builder.adoptBy(ns_current.top());
		}

		void end_file_(Iter first, Iter last) {
			//m_tr.commit();
		}
		void throw_not_type_(Iter first, Iter last) {
		}

		void throw_not_model_(Iter first, Iter last) {
		}

		void throw_not_arg_(Iter first, Iter last) {
		}

		void throw_not_base_(Iter first, Iter last) {
		}

		void throw_not_member_(Iter first, Iter last) {
		}

		void throw_not_ns_(Iter first, Iter last) {
		}

		void throw_not_file_(Iter first, Iter last) {
		}


		str_action begin_type, new_model, end_args, new_arg_name, new_arg_type, new_base, new_member, new_member_type, end_type, new_ns, end_ns, end_file;
		str_action throw_not_type, throw_not_model, throw_not_arg, throw_not_base, throw_not_member, throw_not_ns, throw_not_file;

	private:
		Xirang* m_xr;

        typedef std::pair<std::shared_ptr<NamespaceBuilder>, Namespace> NamespaceBuilderParentPair;
        std::stack<NamespaceBuilderParentPair>  ns_builders;
        
        TypeBuilder tp_builder;
        TypeAliasBuilder ta_builder;

        std::stack<Namespace> ns_current;

		aio::string m_name;
        aio::string m_typeName;
};

template<typename Iter>
class TypeGrammer : public bsc::grammar<TypeGrammer<Iter>>
{
	public:
		typedef TypeLoaderActions<Iter> action_type;
		action_type& act;

		TypeGrammer(action_type& action) : act(action){}

		template<typename ScannerT> class definition
		{
			public:
				definition(const TypeGrammer& self)
				{
					using namespace bsc;

					types_
						= (*namespace_)[self.act.end_file] | eps_p[self.act.throw_not_file]
						;

					namespace_
						= str_p("namespace") >> ns_id_[self.act.new_ns] >> str_p("{") >> *type_ >> (str_p("}")[self.act.end_ns] | eps_p[self.act.throw_not_ns]) 
						;

					type_
						= str_p("type") >> id_[self.act.begin_type] >> (body_ | eps_p[self.act.throw_not_type])
						;

					body_
						= str_p("{") >> items_ >> (str_p("}")[self.act.end_type] | eps_p[self.act.throw_not_type])
						;

					items_
						= 
						  (models_ >> !args_)
						| (!args_ >> !bases_ >> !members_)
						;

					models_
						= str_p("model:") >> id_[self.act.new_model] >> (str_p(";") | eps_p[self.act.throw_not_model])
						;

					args_
						= (str_p("arg:") >> +arg_def_)[self.act.end_args] 
						;

					bases_
						= str_p("base:") >> base_def_
						;

					members_
						= str_p("member:") >> +member_def_
						;

					arg_def_
						= id_[self.act.new_arg_name] >> (!id_)[self.act.new_arg_type] >> (str_p(";") | eps_p[self.act.throw_not_arg])
						;

					base_def_
						= id_[self.act.new_base] >> *(str_p(",") >> base_def_) >> (str_p(";") | eps_p[self.act.throw_not_base])
						;

					member_def_
						= id_[self.act.new_member] >> id_[self.act.new_member_type] >> (str_p(";") | eps_p[self.act.throw_not_member])
						;

					id_
						= lexeme_d[chset_p("a-zA-Z") >> *chset_p("0-9a-zA-Z_")]
						;

					ns_id_
						= lexeme_d[+(str_p(".") >> id_)]
						;

				};

				bsc::rule< ScannerT > types_, namespace_, type_, id_, body_, items_, models_, args_, bases_, members_, arg_def_, base_def_, member_def_,
					ns_id_;

				const bsc::rule< ScannerT >& start() const { return types_; }
		};

};

void TypeLoader::load(const aio::buffer<unsigned char>& buf, Xirang& xr)
{
	//	typedef aio::buffer<unsigned char>::const_iterator Iter;
	typedef const char* Iter;
	TypeLoaderActions<Iter> actions(xr);

	try
	{
		const bsc::parse_info<Iter> info = 
			bsc::parse((Iter)buf.begin(), (Iter)buf.end(), TypeGrammer<Iter>(actions), bsc::space_p);
		if (!info.hit)
		{
			std::cout << "load failed \n"; 
		}

	}
	catch(...)
	{
			std::cout << "load failed \n"; 
	}
}
