Access Token
Field 	Description
access_token 	Your access token
expires_in 	Time in seconds before token expires
scope 	Authorized scope of the token
issued_at 	Date timestamp when the token was issued
status 	Current status of the token -- this is for future use only
refresh_token 	Your refresh token (if enabled)

Balances
Field 	Description
option_short_value 	Value of short option positions
total_equity 	Total account value
account_number 	Account number
account_type 	Type of the account
close_pl 	Gain/Loss of current session's closed positions
current_requirement 	Account's maintenance margin
equity 	Equity value
long_market_value 	Long market value
market_value 	Market value
open_pl 	Total gain/loss of current account's positions
option_long_value 	Value of long option positions
option_requirement 	Account's total option requirement
pending_orders_count 	Count of all pending/open orders
short_market_value 	Short market value
stock_long_value 	Value of long equity positions
total_cash 	Total cash in the account
uncleared_funds 	Cash unavailable for trading in the account
pending_cash 	Amount of cash being held for open orders
If type is "margin"
margin.fed_call 	Amount that the account is in deficit for trades that have occurred but not been paid for
margin.maintenance_call 	Amount that the account is under the minimum equity required to support the current positions
margin.option_buying_power 	Amount of funds available to purchase non-marginable securities
margin.stock_buying_power 	amount of funds available to purchase fully marginable securities
margin.stock_short_value 	Value of short stocks
margin.sweep 	
If type is "cash"
cash.cash_available 	
cash.sweep 	Sweep
cash.unsettled_funds 	Cash that is in the account from recent stock or option sales, but has not yet settled; cash from stock sales occurring during the last 2 trading days or from option sales occurring during the previous trading day
If type is "pdt"
pdt.fed_call 	Amount that the account is in deficit for trades that have occurred but not been paid for
pdt.maintenance_call 	Amount that the account is under the minimum equity required in the account to support the current positions
pdt.option_buying_power 	Amount of funds available to purchase non-marginable securities
pdt.stock_buying_power 	Amount of funds available to purchase fully marginable securities
pdt.stock_short_value 	Value of short stocks

Calendar
Field 	Description
date 	Date
status 	Status of the market, one of: open, closed
description 	Details about the market status
start 	Time the market opens in EST
end 	Time the market closes in EST

Clock
Field 	Description
date 	Current date
description 	Displayable description of the status
state 	Market state, one of: premarket, open, postmarket, closed
timestamp 	UNIX timestamp of the present time
next_change 	Next state change in hours/minutes
next_state 	Next state change

Gainloss
Field 	Description
close_date 	Date the position was closed
cost 	Total cost of the position
gain_loss 	Gain or loss on the position
gain_loss_percent 	Gain or loss represented as percent
open_date 	Date the position was opened
proceeds 	Total amount received for the position
quantity 	Quantity of shares/contracts
symbol 	Symbol of the security held
term 	Term in months position was held

History
Field 	Description
amount 	Value of transaction
date 	Date of event
type 	Type of event that occurred (trade, journal)
description 	Text description of event
commission 	Commission
price 	Price
quantity 	Quantity
symbol 	Symbol
trade_type 	Security type of the trade (Equity, Option)

Historical Data
Field 	Description
date 	Date of the data point
open 	Open price
high 	High price
low 	Low price
close 	Close price
volume 	Volume


Orders
Field 	Description
id 	Unique identifier for the order
type 	Single-leg, One of: market, limit, stop, stop_limit
Multi-leg, One of: market, debit, credit, even
symbol 	Security symbol or underlying security symbol
side 	Equity, One of: buy, buy_to_cover, sell, sell_short
Option, One of: buy_to_open, buy_to_close, sell_to_open, sell_to_close
quantity 	Number of shares or contracts
status 	One of: open, partially_filled, filled, expired, canceled, pending, rejected, error
duration 	One of: day, pre, post, gtc
price 	Limit price
avg_fill_price 	Average fill price
exec_quantity 	Total number of shares/contracts filled
last_fill_price 	Last fill price
last_fill_quantity 	Last fill quantity
remaining_quantity 	Number of shares/contracts remaining
create_date 	Date the order was created
transaction_date 	Date the order was last updated
class 	One of: equity, option, combo, multileg
strategy 	One of: freeform, covered_call, protective_put, strangle, straddle, spread, collar, butterfly, condor, unknown
option_symbol 	OCC option symbol
stop_price 	Stop price
reason_description 	Rejection details
tag 	Order tag if available

Positions
Field 	Description
cost_basis 	Cost of the position
date_acquired 	Date position was acquired (or most recently updated)
id 	Unique position identifier
quantity 	Number of shares/contracts (positive numbers indicate long positions, negative numbers indicate short positions)
symbol 	Security symbol

Search/Lookup
Field 	Description
symbol 	Security symbol
exchange 	Exchange code of the symbol
type 	Type, one of: stock, option, etf, index, mutual_fund
description 	Security detailed name or description

Streaming Responses
Quote

The quote event is issued when a viable quote has been created an exchange. This represents the most current bid/ask pricing available.
Field 	Description
type 	Type of the event
symbol 	Security symbol for the event
bid 	Bid price
bidsz 	Bid size
bidexch 	Bid exchange
biddate 	Bid date
ask 	Ask price
asksz 	Ask size
askexch 	Ask exchange
askdate 	Ask date
Trade

The trade event is sent for all trade events at exchanges. By default, the trade event is filtered to only include valid ticks (removing trade corrections, errors, etc).
Field 	Description
type 	Type of the event
symbol 	Security symbol for the event
exch 	Exchange code reporting the event
price 	Last price available
size 	Size of the trade event
cvol 	Cumulative volume
date 	Trade date
last 	Last price (this is a duplicate of price)
Summary

The summary event is triggered when a market session high, low, open, or close event is triggered.
Field 	Description
type 	Type of the event
symbol 	Security symbol for the event
open 	Open price
high 	High price
low 	Low price
prevClose 	Previous close price
Timesale

Time and Sale represents a trade or other market event with price, like market open/close price, etc. Time and Sales are intended to provide information about trades in a continuous time slice (unlike Trade events which are supposed to provide snapshot about the current last trade). Timesale events are uniquely sequenced.
Field 	Description
type 	Type of the event
symbol 	Security symbol for the event
exch 	Exchange code reporting the event
bid 	Bid price
ask 	Ask price
last 	Last price
size 	Size of the event
date 	Date of the event
seq 	Sequence number
flag 	Event flag Reference
cancel 	Was this a cancel event
correction 	Was this a correction event
session 	Market session type of the event
Tradex

The trade event is sent for all trade events at exchanges. This payload has more accurate information during the pre/post market sessions. If you plan streaming market data during these sessions, you should use this payload over the trade payload.
Field 	Description
type 	Type of the event
symbol 	Security symbol for the event
exch 	Exchange code reporting the event
price 	Last price available
size 	Size of the trade event
cvol 	Cumulative volume
date 	Trade date
last 	Last price (this is a duplicate of price)

Timesale (1min, 5min, 15min)
Field 	Description
time 	Time of the interval
timestamp 	UNIX timestamp of the interval
price 	Last price of the interval
open 	Open price of the interval
high 	High price of the interval
low 	Low price of the interval
close 	Close price of the interval
volume 	Volume of the interval
vwap 	Volume weighted average price of the interval
Timesale (tick)
Field 	Description
time 	Time of the interval
timestamp 	UNIX timestamp of the interval
price 	Last price of the interval
volume 	Volume of the interval

Watchlists
Field 	Description
watchlists 	An array of watchlists
id 	Watchlist's Id
name 	Watchlists's name
public_id 	The watchlist's public Id to be used for sharing

Watchlist
Field 	Description
id 	Watchlist's Id
public_id 	The watchlist's public Id to be used for sharing
name 	Watchlists's name
items 	An array of the watchlist items
symbol 	Item's symbol

